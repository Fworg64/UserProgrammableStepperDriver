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

#define STEPPER_NUMBER_OF_STEPS		8

#define BIT_PLACE			4

#define MS_MAX				0xFF
#define MS_SUB_MAX			9

#define DIRECTION_UP			1
#define DIRECTION_DOWN			0

#define RUNNING_ON			1
#define RUNNING_OFF			0

#define STEPPER_OUTPUT_PINS		(0x0F << BIT_PLACE)

#define STEPS_PER_ROT			(200)

#define STEPPER_DRIVE_FREQ	1000

// clock freq is 250KHz/2500

//#define DEFINE define
#define MAINMENU 0
#define RUNNINGMENU 1
#define SETTINGMENU 2
#define FRAMEUPDATEMS 30 //30MS faster than 30 fps

unsigned char ms = 0;
unsigned char framems=0;

unsigned char screen =MAINMENU;
char mainmenustring[] =    "1.GO XX.XX 3.FFW2.SET RPM  4.FBW";
char runningmenustring[] = "2.STOP                          ";
char settingsmenustring[]= "Curr: XX.XX RPM New :   .  RPM  ";
volatile char runframe =1; 

char inputcar;
char getanotherkey=1;

//eeprom and rpm stuff
struct eeprom_struct eeprom_rpm_high = {.startaddress = 25, .data = 10, .number_of_redundancy = 5};
struct eeprom_struct eeprom_rpm_low  = {.startaddress = 31, .data = 10, .number_of_redundancy = 5};
int eeprpm_setup();
void eeprpm_write(int rpm2write);
char EEPROM_ERROR=0;
int rpm; //initialized from eeprpm_setup
char rpmdisplaychars[5];

int main (void)
{
	USART_init (207); // 9600 baud
	timer_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (8);
	lcd_set_contrast(50);
	keypad_init();
	sei ();	// enable interrupts
	timer_start ();
	lcd_send_string("Phase 1");
	rpm = eeprpm_setup(); //setup rpm in memory from value potentially stored in eemprom. If no valid value is found a default one is written and loaded.
	lcd_send_string("Phase 2");
	
	char updatescreen =0;

	rpmdisplaychars[0] = rpm/1000 + '0';
	rpmdisplaychars[1] = rpm/100 -(rpmdisplaychars[0]-'0')*10 +'0';
	rpmdisplaychars[2] = '.';
	rpmdisplaychars[3] = rpm/10 - (rpmdisplaychars[0]-'0')*100 - (rpmdisplaychars[1]-'0')*10 + '0';
	rpmdisplaychars[4] = rpm - (rpmdisplaychars[0]-'0')*1000 - (rpmdisplaychars[1]-'0')*100 - (rpmdisplaychars[3]-'0')*10 + '0';

	for (char looper=0; looper <5; looper++) mainmenustring[5+looper] = rpmdisplaychars[looper];

	lcd_send_string(mainmenustring);
	lcd_send_string(mainmenustring);

	while (9)
	{ if (runframe) { //only if runframe is true;
		runframe = 0; //wait for isr to reset to 1 after 30ms
		//USART_transmit('b');
		//USART_transmit(getanotherkey ? 'a' : 'f');
		if (getanotherkey) pollKeys(); //call this guy everyframe

		//get input if getanotherkey
		if (isKeyPressed() && getanotherkey) // a key was pressed, but not yet released
		{
		    inputcar = getKey(); //get input from input poller
		    getanotherkey=0; //disable next key fetch
		    //USART_transmit(inputcar);
		    //USART_transmit('c');
			
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
			

			//(inputcar != '\0') ? USART_transmit('n') : USART_transmit(inputcar);
			//(dont) put code here to be run for any input on any screen
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
	//code to run every while
	
	}
	return 0; //this shouldnt execute.
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	if (ms_sub_timer++ >= MS_SUB_MAX){
		if (ms++ >= MS_MAX){
			ms = 0;
		}
		if (++framems>=FRAMEUPDATEMS) {runframe=1; framems =0;}
		ms_sub_timer = 0;
	}
}

int eeprpm_setup()
{
	//initial dummy struct
	EEPROM_ERROR = eeprom_redundant_read(&eeprom_rpm_high); //check here for eeprom errors
	EEPROM_ERROR = eeprom_redundant_read(&eeprom_rpm_low); //also here
	//check for sanity of values
	if ((eeprom_rpm_high.data >= 100) || (eeprom_rpm_low.data >=100)) EEPROM_ERROR = 1;//ERROR, try to write better values
	if (EEPROM_ERROR == EEPROM_IS_CORRUPT)
	{
		//set error flag
		eeprom_rpm_high.data = 10;
		eeprom_rpm_low.data = 10;
		eeprom_redundant_write(eeprom_rpm_low);
		eeprom_redundant_write(eeprom_rpm_high);
		mainmenustring[20] = 'E';
		mainmenustring[21] = 'R';
		mainmenustring[22] = 'R';
		mainmenustring[23] = '0';
		mainmenustring[24] = 'R';
	}

	//convert high low chars to int
	//lcd_send_string("Phase 1");
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



/*
These are definitions to be placed into TCCR1B
typedef enum timer_prescale_enum {
	STOPPED=0, PRESCALE_1 = 1, PRESCALE_8 = 2,
	PRESCALE_64 =3, PRESCALE_256=4,
	PRESCALE_1024 = 5, EXTERNAL_FALLING=6, EXTERNAL_RISING=7
}t_timer_prescale;

*/


