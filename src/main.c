/*
* Authors: Marc Olberding, Austin Oltmans
* Company: CTIPP @ NDSU
* Date: 12/5/16
*
* Rev.History
* 12/7/2016 - Added keypad code (Austin)
* 12/12/2016 - added menu code
* 12/14/2016 -refactored
 code for new driver
*
*/


#define F_CPU		16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD.h"
#include "usci.h"
#include "menu.h"
#include "timer.h"
#include "eeprom.h"
#include "keypad.h"
#include "stepper.h"
#include "button.h"

#define MS_MAX	60000
#define MS_SUB_MAX	9

#define MAINMENU 			0
#define RUNNINGMENU 	1	
#define SETTINGMENU 	2
#define FASTFORWARD 	3
#define FASTBACKWARD 	4
#define FRAMEUPDATEMS 30 //30MS faster than 30 fps

#define FASTSPEED 1875 //clocks for 40 rpm, 7500000/4000

#define NO_FAULT_STATE	0
#define FAULT_STATE			1 

#define BULLET_MAX			5

#define FAULT_MOTOR_OVERHEAT	0x01	
#define FAULT_TOO_MANY_BULLETS 0x02
#define FAULT_STATE_MACHINE		0x04

#define BULLET_COUNTER_PIN	(1 << PE1)
#define WHEEL_FEEDBACK_PIN (1 << PE2)

#define DEBOUNCE_COMPARE	0xFF00

#define MICROSTEPS_PER_STEP	63
#define STEPS_PER_REVOLUTION 200

#define STEPS_PER_MOTOR_STATE (STEPS_PER_REVOLUTION/8)

#define STATE_NO_OP								0
#define STATE_WAITING_FOR_BULLETS	1
#define STATE_TURNING_WHEEL_WAITING_FOR_SWITCH	2
#define STATE_TURNING_WHEEL_WAITING_FOR_SECOND_SWITCH	3
#define STATE_TURNING_WHEEL_CENTERING						4


unsigned int ms = 0;
volatile char runframe =1;
unsigned char fault = 0;
unsigned char bulletcount = 0;
unsigned char changedir = 0;
unsigned long int wheel_index = 0;
unsigned long int goal= 0;
char*screenptr;

struct button_struct bulletcounter_button = {.state = 0, .laststate = 0};
struct button_struct wheelfeedback_button = {.laststate = 0, .state = 0};


struct stepper_driver_struct stepper1 = {.stepperport = &PORTC, .stepperreadport = &PINC, .dirpinmask = 1<<PC1, .faultpinmask = 1<<PC0,.enablepinmask =1<<PC2, .togglecomparetime = 3750, .dir =1, .internaltimer=0, .enable=0};

void start_stepper (struct stepper_driver_struct *step);
void stop_stepper (struct stepper_driver_struct *step);
void binary_to_string (unsigned char bin, char *str);

void delay(int delayer){
	while (delayer--){
		_delay_ms (1);
	}
}

char state = STATE_NO_OP;
char updatescreen =1;
	char statewaitingstring[] = "Waiting for bullets";
	char statewaitingswitch[] = "Waiting for switch";
	char statewaitingswitch2[] = "Waiting switch 2";
	char statewaitingcentering[] = "centering";
int main (void)
{
  DDRC |=  0b00000110;
  PORTC |= 0b00000001;
  fault = 0;
  DDRE = 0x00;
  PORTE = 0x7;			// pullup resistors on porte
	char inputcar;
	char getanotherkey=1;
	char mainmenustring[33] =    "1.Start";
	char runningmenustring[33] = "2.STOP";
	char faultstring[33] = "FAULT";
	screenptr = mainmenustring;
	USART_init (207); // 9600 baud
	timer_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (2);
	lcd_set_contrast(50);
	keypad_init();
	stop_stepper (&stepper1);
	delay (10000);
	sei ();	// enable interrupts
	lcd_reset ();
	stepper1.togglecomparetime = RPMtofromCompareTime(1500);
	OCR1A = stepper1.togglecomparetime;
	while (9)
	{ 
		if (runframe) { //only if runframe is true;
			runframe = 0; //wait for isr to reset to 1 after 30ms
			pollKeys(); //call this guy everyframe

			//get input if getanotherkey
			if (isKeyPressed() && getanotherkey) // a key was pressed, but not yet released
			{
				  inputcar = getKey(); //get input from input poller
				  getanotherkey=0; //disable next key fetch

				if (screenptr == mainmenustring){
						if (inputcar == '1')
						{
							screenptr = runningmenustring;
							state = STATE_WAITING_FOR_BULLETS;
							updatescreen =1; //be sure to call this guy if you want to see anything
						}
				}
				else if (screenptr == runningmenustring){
						if (inputcar == '2')
						{
		         	stop_stepper (&stepper1);
		          screenptr = mainmenustring;
							updatescreen=1;  //be sure to call this guy if you want to see anything
							state = STATE_NO_OP;
							//stop mot0r
						}
						//spin motor at rpm here
				}
				//(dont) put code here to be run for any input on any screen
			}

			if (!isKeyPressed() && wasKeyReleased() && !getanotherkey) // key pressed before, need to check if it was released.
			{
				  clearKey();
				  getanotherkey =1;
				//shouldnt do much more here, unless you need to do something on a key release
			}

			//display output based on screen and anything else
			if (updatescreen)
			{
				updatescreen =0; //screen update handled, wait for next time,, next time
				lcd_send_string (screenptr);
				if (screenptr == faultstring){
					cli ();
					while (1);
				}
			}
		}
		//code to run every while
		if (fault != 0){
			stop_stepper (&stepper1);
			faultstring[5] = ' ';
			binary_to_string (fault, faultstring+6);
			screenptr = faultstring;
			updatescreen =1;
		}
		
		switch (state){
			case STATE_NO_OP:
				break;
			case STATE_WAITING_FOR_BULLETS:// 1. wait for bullets
				if (button_struct_check_state (&bulletcounter_button) == BUTTON_POS_EDGE){
					bulletcount++;
					if (bulletcount >= BULLET_MAX){
						bulletcount = 0;
						state = STATE_TURNING_WHEEL_WAITING_FOR_SWITCH;
						updatescreen = 1;
						screenptr = statewaitingswitch;
						start_stepper (&stepper1);
					}
				}
				break;
			case STATE_TURNING_WHEEL_WAITING_FOR_SWITCH:
				if (button_struct_check_state (&wheelfeedback_button) == BUTTON_NEG_EDGE){
						state = STATE_TURNING_WHEEL_WAITING_FOR_SECOND_SWITCH;
						screenptr = statewaitingswitch2;
						updatescreen = 1;
						wheel_index = 0;
				}
				break;
			case STATE_TURNING_WHEEL_WAITING_FOR_SECOND_SWITCH:
				if (button_struct_check_state (&wheelfeedback_button) == BUTTON_POS_EDGE){
						goal = wheel_index >> 1;
						state = STATE_TURNING_WHEEL_CENTERING;
						screenptr = statewaitingcentering;
						updatescreen = 1;
						changedir = 1;
				}
				break;
			case STATE_TURNING_WHEEL_CENTERING:
				if (wheel_index <= goal){
					state = STATE_WAITING_FOR_BULLETS;
					*stepper1.stepperport ^= stepper1.dirpinmask;
					screenptr = statewaitingstring;
					updatescreen =1;
					stop_stepper (&stepper1);
				}
				break;
			default:
					fault |= FAULT_STATE_MACHINE;
					updatescreen = 1;
				break;
			// 2. turn wheel (reset bullet count)
		}
		
	
	}
	return 0; //this shouldnt execute.
}


ISR (TIMER1_COMPA_vect)
{

    if ( stepper1.enable) {
    	OCR1A = stepper1.togglecomparetime;
			if (state == STATE_TURNING_WHEEL_WAITING_FOR_SECOND_SWITCH){
				wheel_index++;
			} else if (state == STATE_TURNING_WHEEL_CENTERING){
				if (changedir){
					*stepper1.stepperport ^= stepper1.dirpinmask;
					changedir = 0;
				}
				wheel_index--;
			}		
    }
    else
    {
        PORTD &= ~(1<<PD5); //turn off power to the step pin
    }
    if ((*(stepper1.stepperreadport) & (stepper1.faultpinmask)) == 0) //bad, overheat or something when pin is low
    {
				stop_stepper (&stepper1);
        fault |= FAULT_MOTOR_OVERHEAT;
        PORTD &= ~(1<<PD5); //turn off power to the step pin
    }
}

ISR (TIMER0_COMP_vect)
{
		static unsigned char framems = 0;
		static unsigned int buttonreg = 0xFFFF;
		static unsigned int buttonreg2 = 0xFFFF;
    if (ms++ >= MS_MAX) {ms=0;}
    if (++framems>=FRAMEUPDATEMS) {runframe=1; framems =0;}
    
    buttonreg <<= 1;
    if (PINE & BULLET_COUNTER_PIN){
    	buttonreg |= 1;
		}
		bulletcounter_button.state = buttonreg <= DEBOUNCE_COMPARE;
		
		buttonreg2 <<=1;
		if (PINE & WHEEL_FEEDBACK_PIN){
			buttonreg2 |= 1;
		}
		wheelfeedback_button.state = buttonreg2 <= DEBOUNCE_COMPARE;		
}

void start_stepper (struct stepper_driver_struct *step){
	*(step->stepperport) |= step->enablepinmask;
	stepper1.enable = 1;
	timer_start ();
}

void stop_stepper (struct stepper_driver_struct *step){
	*(step->stepperport) &= ~step->enablepinmask;
	stepper1.enable = 0;
	timer_stop ();
}

void binary_to_string (unsigned char bin, char *str){
	unsigned char mask = 0x80;
	unsigned char ctr;
	for (ctr = 0; ctr < 8; ctr++){
		str[ctr] = '0' + ((bin & mask) != 0);
		mask >>=1;
	}
	str[ctr] = '\0';
}



