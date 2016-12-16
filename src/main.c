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

// 1. implement debouncing on interrupt switches done
// 2. implement counting on debounced interrupt switches done
// 3. implement valve timing done
// 4. implement seperate main loops for normal operation and setup operation done
// 5. implement mux line for second LCD screen. ???? not necessary

unsigned char belt_in_position = 1;
unsigned char buttonstate = 0;

#define MAINMENU 			0
#define RUNNINGMENU 	1
#define SETTINGMENU 	2
#define ENTERMENU			3
#define FRAMEUPDATEMS 30 //30MS faster than 30 fps

#define START_CHAR		('#')
#define STOP_CHAR			('*')

#define DELAY_EEPROM_OFFSET	0
#define ONTIME_EEPROM_OFFSET 1

unsigned int ms = 0;
volatile char runframe =1;

void fill_with (char *buffer, unsigned char length, char);
int string_to_int (char *string);
void int_to_string (char*buff, int number);

int main (void)
{
	unsigned char bullet_count = 5;
	char mainmenustring[33] =    "1 drop  2 split 3 push  4 run";
	char settingsmenustring[33] =    "1 delay 2 Ontime3 Main Menu         ";
	char runningmenustring[33]    =    "1. stop";
	char entermenustring[33]      =    "";
	char inputcar;
	
 	struct eeprom_16_bit valvedropper_e[2];
 	struct eeprom_16_bit valvepusher_e[2];
 	struct eeprom_16_bit valvesplitter_e[2];
 	
	eeprom_16_bit_init (valvedropper_e+ONTIME_EEPROM_OFFSET, 0, 3);
	eeprom_16_bit_init (valvedropper_e+DELAY_EEPROM_OFFSET, 6, 3);
	eeprom_16_bit_init (valvepusher_e+ONTIME_EEPROM_OFFSET, 12, 3);
	eeprom_16_bit_init (valvepusher_e+DELAY_EEPROM_OFFSET, 18, 3);
	eeprom_16_bit_init (valvesplitter_e+ONTIME_EEPROM_OFFSET, 24, 3);
	eeprom_16_bit_init (valvesplitter_e+DELAY_EEPROM_OFFSET, 30, 3);
	
	unsigned char screen =MAINMENU;
	
	fill_with (entermenustring, 32, ' ');
	entermenustring[33] = '\0';
	unsigned char last_buttonstate = 0;
	unsigned char stop_signal = 0;
	unsigned char mode = MODE_SETUP;
	unsigned char state = STATE_SETTING_BELT;
	unsigned char rpminputindex = 0;
	char rpminputbuff[5];
	char getanotherkey=1;
	char updatescreen =0;
	struct valve *v_ptr;
	struct eeprom_16_bit *ee_ptr;
	unsigned int *ui_ptr;
	
	PORTC = 0;
	DDRC = 0xFF;
	struct valve splitter, dropper, pusher;
	
	// load stuff from eeprom
	eeprom_16_bit_read (valvedropper_e+ONTIME_EEPROM_OFFSET);
	eeprom_16_bit_read (valvedropper_e+DELAY_EEPROM_OFFSET);
	eeprom_16_bit_read (valvepusher_e+ONTIME_EEPROM_OFFSET);
	eeprom_16_bit_read (valvepusher_e+DELAY_EEPROM_OFFSET);
	eeprom_16_bit_read (valvesplitter_e+ONTIME_EEPROM_OFFSET);
	eeprom_16_bit_read (valvesplitter_e+DELAY_EEPROM_OFFSET);
		
	valve_init (&splitter, &PORTC, 7, valvesplitter_e[ONTIME_EEPROM_OFFSET].data , valvesplitter_e[DELAY_EEPROM_OFFSET].data);
	valve_init (&dropper, &PORTC, 6, valvedropper_e[ONTIME_EEPROM_OFFSET].data, valvedropper_e[DELAY_EEPROM_OFFSET].data);
	valve_init (&pusher, &PORTC, 5, valvepusher_e[ONTIME_EEPROM_OFFSET].data, valvepusher_e[ONTIME_EEPROM_OFFSET].data);
	
	USART_init (207); // 9600 baud
	timer_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (2);
	lcd_set_contrast(50);
	keypad_init();
	
	unsigned int offset;
	sei ();	// enable interrupts
	timer_start ();
	for (int awful=0; awful<30000;awful++) for (int awfuler =0; awfuler<10;awfuler++);
	for (int awful=0; awful<30000;awful++) for (int awfuler =0; awfuler<100;awfuler++);
	updatescreen = 1;
	
	while (1)
	{
			if (runframe) 
			{ //only if runframe is true;
				runframe = 0; //wait for isr to reset to 1 after 30ms
				pollKeys(); //call this guy everyframe

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
									v_ptr = &dropper;
									ee_ptr = valvedropper_e;
									screen = SETTINGMENU;
									updatescreen  = 1;	
								}
								if (inputcar == '2')
								{
									v_ptr = &splitter;
									ee_ptr = valvesplitter_e;
									screen = SETTINGMENU;
									updatescreen =1;
								}
								if (inputcar =='3')
								{
									v_ptr = &pusher;
									ee_ptr = valvepusher_e;
									screen = SETTINGMENU;
									updatescreen = 1;
								}
								if (inputcar =='4')
								{
									screen = RUNNINGMENU;
									mode = MODE_RUNNING;
									stop_signal = 0;
									updatescreen=1;
								}
				        break;
				     case SETTINGMENU:
								if (inputcar == '1'){
									ui_ptr = &(v_ptr->starttime);
									offset = DELAY_EEPROM_OFFSET;
									int_to_string (entermenustring+16, ee_ptr[offset].data);
									screen = ENTERMENU;
									updatescreen = 1;
								} else if (inputcar == '2'){
									ui_ptr = &(v_ptr->ontime);
									offset = ONTIME_EEPROM_OFFSET;
									int_to_string (entermenustring+16, ee_ptr[offset].data);
									screen = ENTERMENU;
									updatescreen = 1;
								} else if (inputcar == '3'){
									screen = MAINMENU;
									updatescreen = 1;
								}
								break;
							case RUNNINGMENU:
								if (inputcar == '1'){ 
									stop_signal = 1;
									screen = MAINMENU;
									updatescreen=1;  //be sure to call this guy if you want to see anything
								}
								break;
								//stop mot0r
							case ENTERMENU:
								if (inputcar != '\0')
								{
									if (inputcar == STOP_CHAR){
										// discard changes
										rpminputindex = 0;
										fill_with (entermenustring, 16, ' ');
										screen = SETTINGMENU;
										updatescreen = 1;
									}
									else if (inputcar == START_CHAR){
									 // save changes
									 if (rpminputindex >= 4){
										 *ui_ptr = string_to_int (rpminputbuff);
										  eeprom_16_bit_write (&ee_ptr[offset], *ui_ptr);
										 	fill_with (entermenustring, 16, ' ');
										 	screen = SETTINGMENU;
											rpminputindex = 0;
											updatescreen = 1;
										}	
									}
									else{
										if (rpminputindex < 4){
											rpminputbuff[rpminputindex] = inputcar;
											entermenustring[rpminputindex] = inputcar;
											rpminputindex++;
											updatescreen=1;
										}
									}
								}
								break;
							default:
								lcd_send_string ("hell");
								while (1);
								
						}
				}

				if (!isKeyPressed() && wasKeyReleased() && !getanotherkey) // key pressed before, need to check if it was released.
				{
						clearKey();
						getanotherkey =1;
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
							case SETTINGMENU:
								lcd_send_string(settingsmenustring);
								break;
							case ENTERMENU:
								lcd_send_string(entermenustring);
								break;
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
			if (stop_signal == 0){
				switch (state){
					case STATE_IDLE:
						break;
					case STATE_SETTING_BELT:
						if (belt_in_position && (bullet_count == 5))
						{
							valve_schedule_in_ms (&splitter, MS_MAX, ms);
							state = STATE_SPLIT;
							//bullet_count = 0;
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
							valve_schedule_in_ms (&pusher, MS_MAX, ms);
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
			else if (stop_signal != 0){
				// stop signal received
				if (pusher.state == VALVE_STATE_UNSCHEDULED){
					if (splitter.state == VALVE_STATE_UNSCHEDULED){
						if (dropper.state == VALVE_STATE_UNSCHEDULED){
							mode = MODE_SETUP;
							stop_signal = 0;
							state = STATE_SETTING_BELT;
						}
					}
				}
			}
		}
	}
	return 0; //this shouldnt execute.
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	static unsigned int buttonreg = 0xFFFF;
	static unsigned int framems= 0;
	if (ms_sub_timer++ >= MS_SUB_MAX){ //a ms has passed
        	ms_sub_timer = 0;
		if (ms++ >= MS_MAX){ //reset after toomany ms
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


void fill_with (char *buffer, unsigned char length, char to_fill ){
	unsigned char ctr;
	for (ctr = 0; ctr < length; ctr++){
		buffer[ctr] = to_fill;
	}
}

int string_to_int (char *buff){
	return((buff[0]-'0')*1000 + (buff[1] - '0')*100 + (buff[2]- '0')*10 + (buff[3] - '0'));
}

unsigned int get_digit (unsigned int number, unsigned int digit){
	while (digit - 1){
		digit--;
		number /= 10;
	}
	return (number % 10);
}

void int_to_string (char *buff, int number){
	buff[0] = get_digit (number, 4) + '0';
	buff[1] = get_digit (number,3) + '0';
	buff[2] = get_digit (number,2) + '0';
	buff[3] = get_digit (number, 1) + '0';
}

