/*
* Authors: Marc Olberding, Austin Oltmans
* Company: CTIPP @ NDSU
* Date: 12/5/16
*
* Rev.History
* 12/7/2016 - Added keypad code (Austin)
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "LCD.h"
#include "usci.h"
#include "menu.h"
#include "timer.h"
#include "eeprom.h"
#include "keypad.h"
#include "valve.h"

#define MS_MAX				0xFFF0
#define MS_SUB_MAX			99

#define MODE_SETUP			0
#define MODE_RUNNING			1

#define STATE_IDLE			0
#define STATE_SETTING_BELT		1
#define STATE_SPLIT			2
#define STATE_DROPPING			3
#define STATE_PUSHING			4

#define BULLET_COUNT_MAX		5
#define DEBOUNCE_CMP_VAL		0xFF00

#define MAINMENU 			0
#define RUNNINGMENU			1
#define SETTINGMENU 			2
#define FRAMEUPDATEMS 			30 //30MS faster than 30 fps

// things we need to do:
// 1. implement debouncing on interrupt switches done
// 2. implement counting on debounced interrupt switches done
// 3. implement valve timing done
// 4. implement seperate main loops for normal operation and setup operation done
// 5. implement mux line for second LCD screen. ???? not necessary

unsigned int ms = 0;
unsigned char belt_in_position = 1;
unsigned char bullet_count = 0;
unsigned char buttonstate = 0;

unsigned char framems=0;
unsigned char screen =MAINMENU;
char mainmenustring[] =    "1.GO XX.XX 3.FFW2.SET RPM  4.FBW";
char runningmenustring[] = "2.STOP                          ";
char settingsmenustring[]= "Curr: XX.XX RPM New :   .  RPM  ";
volatile char runframe =1; 
char inputcar;

char EEPROM_ERROR=0;
int rpm; //initialized from eeprpm_setup
char rpmdisplaychars[5];

//eeprom and rpm stuff
struct eeprom_struct eeprom_rpm_high = {.startaddress = 25, .data = 10, .number_of_redundancy = 5};
struct eeprom_struct eeprom_rpm_low  = {.startaddress = 30, .data = 10, .number_of_redundancy = 5};


int eeprpm_setup();
void eeprpm_write(int rpm2write);

int main (void)
{
	unsigned char last_buttonstate = 0;
	unsigned char mode = MODE_SETUP;
	unsigned char state = STATE_SETTING_BELT;
	char tempindex=0;
	char getanotherkey=1;
	char updatescreen =0;
	PORTC = 0;
	DDRC = 0xFF;
	struct valve splitter, dropper, pusher;
	valve_init (&splitter, &PORTC, 7, 1000, 1000);
	valve_init (&dropper, &PORTC, 6, 200, 200);
	valve_init (&pusher, &PORTC, 5, 300, 300);

	USART_init (207);	 // 9600 baud
	timer_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (2);
	lcd_set_contrast(50);
	keypad_init();
	//rpm = eeprpm_setup(); //setup rpm in memory from value potentially stored in eemprom. If no valid value is found a default one is written and loaded.	

	rpmdisplaychars[0] = rpm/1000 + '0';
	rpmdisplaychars[1] = rpm/100 -rpmdisplaychars[0] +'0';
	rpmdisplaychars[2] = '.';
	rpmdisplaychars[3] = rpm/10 - rpmdisplaychars[0] - rpmdisplaychars[1] + '0';
	rpmdisplaychars[4] = rpm - rpmdisplaychars[0] - rpmdisplaychars[1] - rpmdisplaychars[3] + '0';
	lcd_send_string(mainmenustring);
	
	while (1)
	{
		if (mode == MODE_RUNNING){
			if (runframe) 
			{ //only if runframe is true;
				runframe = 0; //wait for isr to reset to 1 after 30ms
				if (getanotherkey) 
				{
					pollKeys(); //call this guy everyframe
				}

				//get input if getanotherkey
				if (isKeyPressed() && getanotherkey) // a key was pressed, but not yet released
				{
					inputcar = getKey(); //get input from input poller
					getanotherkey=0; //disable next key fetch			
					switch (screen) //respond to keypress based on current screen
					{
						case MAINMENU:
							if (inputcar == '1')
							{
								screen = RUNNINGMENU;
								updatescreen =1; //be sure to call this guy if you want to see anything
							}
							break;
						case RUNNINGMENU:
							if (inputcar == '2')
							{
				                screen = MAINMENU;
								updatescreen=1;  //be sure to call this guy if you want to see anything
							}
							break;
					}
				}

				if (wasKeyReleased() && !getanotherkey) // key pressed before, need to check if it was released.
				{
					clearKey();
					getanotherkey =1;
					//shouldnt do much more here, unless you need to do something on a key release
				}

				//display output based on screen and anything else
				if (updatescreen)
				{
					updatescreen =0; //screen update handled, wait for next time,, next time
					switch (screen)
					{
						case MAINMENU:
							lcd_send_string(mainmenustring);
							break;
						case RUNNINGMENU:
							lcd_send_string(runningmenustring);
							break;
					}
				}
			}
		}
		if (mode == MODE_RUNNING)
		{
			// state machine description:
			// 1. move belt until feeler is felt
			// 2. count 5 bullets 
			// 3. fire the splitter
			// 4. schedule the turn off for the splitter
			// 5. schedule the turn on for the dropper valve
			// 6. wait for the turn on for the dropper valve
			// 7. schedule the turn on for the pusher
			// 8. wait for the pusher to turn on
			// 9. schedule pusher turn off
			// 10. wait for pusher turn off
			valve_eval (&splitter, ms);
			valve_eval (&dropper, ms);
			valve_eval (&pusher, ms);
			switch (state){
				case STATE_IDLE:
					break;
				case STATE_SETTING_BELT:
					if (belt_in_position && (bullet_count == 5))
					{
						valve_schedule_in_ms (&splitter, MS_MAX, ms);
						state = STATE_SPLIT;
						bullet_count = 0;
					} else 
					{
						// count bullets and move belt here
						if (buttonstate)
						{
							if (last_buttonstate == 0){
								bullet_count++;
								last_buttonstate = 1;
							}
						} else {
							if (last_buttonstate == 1){
								last_buttonstate = 0;
							}
						}
						// move belt here
					}
					break;
				case STATE_SPLIT:	// if we are splitting the bullets
					if (splitter.state == VALVE_STATE_WAITING_FOR_OFF){
						valve_schedule_in_ms (&dropper, MS_MAX, ms);
						state = STATE_DROPPING;
					}
					break;
				case STATE_DROPPING:
					if (dropper.state == VALVE_STATE_WAITING_FOR_OFF){
						valve_schedule_in_ms (&pusher, MS_MAX,ms);
						state = STATE_PUSHING;
					}
					break;
				case STATE_PUSHING:
					if (pusher.state == VALVE_STATE_WAITING_FOR_OFF){
						state = STATE_SETTING_BELT;
					}
					break;
				default:
					//while (1); // nightmare
					break;
					
			}
		}
	//code to run every while
	}
	return 0; //this shouldnt execute.
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	static unsigned int buttonreg = 0xFFFF;
	if (ms_sub_timer++ >= MS_SUB_MAX){
		if (ms++ >= MS_MAX){
			ms = 0;
		}
		if (++framems>=FRAMEUPDATEMS) {runframe=1; framems =0;}
		ms_sub_timer = 0;
		buttonreg <<= 1;
		if (PIND & (1 <<PD6)){
			buttonreg |= 1;
		}
		buttonstate = buttonreg <= DEBOUNCE_CMP_VAL;
	}
	
}

int eeprpm_setup()
{
	//initial dummy struct
	EEPROM_ERROR = eeprom_redundant_read(&eeprom_rpm_high); //check here for eeprom errors
	EEPROM_ERROR = eeprom_redundant_read(&eeprom_rpm_low); //also here
	//check for sanity of values
	if (eeprom_rpm_high.data > 99 || eeprom_rpm_low.data >99) EEPROM_ERROR = 1;//ERROR, try to write better values
	if (EEPROM_ERROR)
	{
		//set error flag
		eeprom_rpm_high.data = 10;
		eeprom_rpm_low.data = 10;
		eeprom_redundant_write(eeprom_rpm_low);
		eeprom_redundant_write(eeprom_rpm_high);
	}

	//convert high low chars to int
	return eeprom_rpm_high.data * 100 + eeprom_rpm_low.data;

}

void eeprpm_write(int rpm2write)
{
	//split rpm2write into high and low
	eeprom_rpm_high.data = (char) (rpm2write/100);
	eeprom_rpm_low.data = (char) (rpm2write - eeprom_rpm_high.data*100);
	eeprom_redundant_write(eeprom_rpm_high);
	eeprom_redundant_write(eeprom_rpm_low);

}
