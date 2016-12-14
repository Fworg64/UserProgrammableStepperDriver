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

#define STEPPER_NUMBER_OF_STEPS		8

#define BIT_PLACE			4

#define MS_MAX				0xFFF0
#define MS_SUB_MAX			99

#define DIRECTION_UP			1
#define DIRECTION_DOWN			0

#define RUNNING_ON			1
#define RUNNING_OFF			0

#define STEPPER_OUTPUT_PINS		(0x0F << BIT_PLACE)

#define STEPS_PER_ROT			(200)

#define STEPPER_DRIVE_FREQ	10000

// clock freq is 250KHz/2500


#define MODE_SETUP	0
#define MODE_RUNNING	1


// things we need to do:
// 1. implement debouncing on interrupt switches
// 2. implement counting on debounced interrupt switches
// 3. implement valve timing
// 4. implement seperate main loops for normal operation and setup operation
// 5. implement mux line for second LCD screen. ????

unsigned int ms = 0;
void port_init (void);

// portd0 is interrupt 0
//
//buffer for text stuff
char mycars[33] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

#define STATE_IDLE			0
#define STATE_SETTING_BELT	1
#define STATE_SPLIT			2
#define STATE_DROPPING		3
#define STATE_PUSHING		4

#define BULLET_COUNT_MAX	5

#define FRAME_NACK			0
#define FRAME_ACK			1


#define DEBOUNCE_CMP_VAL	0xFF00

unsigned char belt_in_position = 1;
unsigned char bullet_count = 0;
unsigned char buttonstate = 0;

int main (void)
{
	unsigned char last_buttonstate = 0;
	unsigned char mode = MODE_RUNNING;
	unsigned char state = STATE_SETTING_BELT;
	char tempindex=0;
	char getanotherkey=1;
	struct valve splitter, dropper, pusher;
	DDRF = 0xFF;
	DDRD = 0;
	PORTD = 1<<PD6;
	valve_init (&splitter, &PORTF, 7, 1000, 1000);
	valve_init (&dropper, &PORTF, 6, 200, 200);
	valve_init (&pusher, &PORTF, 5, 300, 300);
	USART_init (103); // 9600 baud
	timer_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (8);
	lcd_set_contrast(50);
	//lcd_send_string ("Hey there");
	//keypad_init();
	sei ();	// enable interrupts
	valve_schedule_in_ms (&splitter, MS_MAX, ms);
	timer_start ();
	while (3)
	{
		if (mode == MODE_SETUP){
		/*if (getanotherkey) pollKeys(ms); //call this guy everyframe

		if (wasKeyPressed() && !wasKeyReleased() && getanotherkey) // a key was pressed, but not yet released
		{
		    mycars[tempindex] = getKey();
		    getanotherkey=0; //this key has been got
		    if (++tempindex>30) tempindex =0;
		    lcd_send_string(mycars);
		}
		if (wasKeyPressed() && !wasKeyReleased() && !getanotherkey) // key pressed, need to check if it was released.
		{
		    pollKeys(ms);
		    if (wasKeyReleased()) //key released, ok to get another
		    {
		        clearKey();
		        getanotherkey =1;
		    }
		}*/
		} else if (mode == MODE_RUNNING)
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
				//valve_eval (&dropper, ms);
				//valve_eval (&pusher, ms);
				/*switch (state){
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
					
				}	*/	
		}
	}
	return 0;
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	static unsigned int buttonreg = 0xFFFF;
	static unsigned char timer2 = 0;
	if (ms_sub_timer++ >= MS_SUB_MAX){
		if (ms++ >= MS_MAX){
			ms = 0;
		}
		ms_sub_timer = 0;
		buttonreg <<= 1;
		if (PIND & (1 <<PD6)){
			buttonreg |= 1;
		}
		buttonstate = buttonreg <= DEBOUNCE_CMP_VAL;
	}
	
}

/*
These are definitions to be placed into TCCR1B
typedef enum timer_prescale_enum {
	STOPPED=0, PRESCALE_1 = 1, PRESCALE_8 = 2,
	PRESCALE_64 =3, PRESCALE_256=4,
	PRESCALE_1024 = 5, EXTERNAL_FALLING=6, EXTERNAL_RISING=7
}t_timer_prescale;

*/


