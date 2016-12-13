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

unsigned char ms = 0;

unsigned char screen =MAINMENU;
char mainmenustring[] =    "1.GO XX.XX 3.FFW2.SET RPM  4.FBW";
char runningmenustring[] = "1.STOP                          ";
char settingsmenustring[]= "Curr: XX.XX RPM New :   .  RPM  ";

char inputcar;
char getanotherkey=1;

//eeprom and rpm stuff
struct eeprom_struct eeprom_rpm_high = {.startaddress = 25, .data = 10, .number_of_redundancy = 5};
struct eeprom_struct eeprom_rpm_low  = {.startaddress = 30, .data = 10, .number_of_redundancy = 5};
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
	rpmdisplaychars[1] = rpm/100 -rpmdisplaychars[0] +'0';
	rpmdisplaychars[2] = '.';
	rpmdisplaychars[3] = rpm/10 - rpmdisplaychars[0] - rpmdisplaychars[1] + '0';
	rpmdisplaychars[4] = rpm - rpmdisplaychars[0] - rpmdisplaychars[1] - rpmdisplaychars[3] + '0';
	lcd_send_string("Phase 3");
	updatescreen=1;

	while (3)
	{

		if (getanotherkey) pollKeys(ms); //call this guy everyframe

		//get input
		if (wasKeyPressed() && !wasKeyReleased() && getanotherkey) // a key was pressed, but not yet released
		{
		    inputcar = getKey();
		    getanotherkey=0;

			//this key has been got
			switch (screen)
			{
				case MAINMENU:
					if (inputcar == '1')
					{
						screen = RUNNINGMENU;
						updatescreen=1;
					}
					if (inputcar!= '\0') 
					{
						mainmenustring[4] = inputcar;
						updatescreen=1;
					}
					break;
				case RUNNINGMENU:
					if (inputcar == '1')
					{
                        			screen = MAINMENU;
						updatescreen=1;
					}
					break;
			}
		}

		if (wasKeyPressed() && !wasKeyReleased() && !getanotherkey) // key pressed, need to check if it was released.
		{
		    pollKeys(ms);
		    if (wasKeyReleased()) //key released, ok to get another
		    {
		        clearKey();
		        getanotherkey =1;
		    }
		}

		//display output;
		if (updatescreen)
		{
			updatescreen =0;
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
	return 0;
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	if (ms_sub_timer++ >= MS_SUB_MAX){
		if (ms++ >= MS_MAX){
			ms = 0;
		}
		ms_sub_timer = 0;
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


