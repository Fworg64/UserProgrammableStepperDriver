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

#define DEFINE define
#DEFINE MAINMENU =0;
#DEFINE RUNNINGMENU =1;
#DEFINE SETTINGMENU =2;

const char stepperarray[STEPPER_NUMBER_OF_STEPS]=
{
0x0C << BIT_PLACE, 0x04 << BIT_PLACE, 0x06 << BIT_PLACE, 0x02 << BIT_PLACE,
0x03 << BIT_PLACE, 0x01 << BIT_PLACE, 0x09 << BIT_PLACE, 0x08 << BIT_PLACE
};

unsigned char ms = 0;

unsigned char screen =0;
char mainmenustring[] =    "1.GO XX.XX 3.FFW2.SET RPM  4.FBW";
char runningmenustring[] = "1.STOP                          ";
char settingsmenustring[]= "Curr: XX.XX RPM New :   .  RPM  ";

void port_init (void);

struct stepper {
	unsigned long ctr; // 0
	unsigned long cmp; // set by set rpm
	unsigned char enabled; // enable
	unsigned char direction; //DIRECTION_UP OR DONWN
	unsigned long number_of_rots; //0 //ACTUALLY NUMBER OF STEPS
	unsigned char index; //SET TO ZERO
};

struct stepper stepper_1 = {.ctr = 0, .enabled = 0, .direction = DIRECTION_UP, .number_of_rots = 0, .index = 0};

void set_rpm (struct stepper *stepper, unsigned int rpm);

// portd0 is interrupt 0
//
//buffer for text stuff
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
	USART_init (103); // 9600 baud
	timer_init ();
	port_init ();
	set_rpm (&stepper_1, 60);
	stepper_1.enabled = RUNNING_OFF;
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (8);
	lcd_set_contrast(50);
	keypad_init();
	sei ();	// enable interrupts
	timer_start ();
	rpm = epprpm_setup(); //setup rpm in memory from value potentially stored in eemprom. If no valid value is found a default one is written and loaded.
	set_rpm(&stepper_1, rpm)
	
	rpmdisplaychars[0] = rpm/1000 + '0';
	rpmdisplaychars[1] = rpm/100 -rpmdisplaychars[0] +'0';
	rpmdisplaychars[2] = '.';
	rpmdisplaychars[3] = rpm/10 - rpmdisplaychars[0] - rpmdisplaycharsp[1] + '0';
	rpmdisplaychars[4] = rpm - rpmdisplaychars[0] - rpmdisplaycharsp[1] - rpmdisplaychars[3] + '0';
	
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
					if (inputcar = '1') 
					{
						stepper_1.enabled = 1;
						screen = RUNNINGMENU;
					}
					break;
				case RUNNINGMENU:
					if (inputcar = '1') stepper_1.enabled =0;
					break;
			}
			
		    //if (++tempindex>30) tempindex =0;
		    //lcd_send_string(mycars);
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
	return 0;
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	if (stepper_1.enabled == RUNNING_ON){
		if (stepper_1.ctr++ >= stepper_1.cmp){
			stepper_1.ctr = 0;
			PORTF = stepperarray[stepper_1.index];
			if (stepper_1.direction == DIRECTION_UP)
			{
				stepper_1.index++;
				stepper_1.number_of_rots++;
				if (stepper_1.index >= STEPPER_NUMBER_OF_STEPS)
				{
					stepper_1.index = 0;
				}
			} 
			else 
			{
				if (stepper_1.index == 0)
				{
					stepper_1.index = STEPPER_NUMBER_OF_STEPS;
				} 
				stepper_1.index--;
				stepper_1.number_of_rots++;
			}
		}
	}
	if (ms_sub_timer++ >= MS_SUB_MAX){
		if (ms++ >= MS_MAX){
			ms = 0;
		}
		ms_sub_timer = 0;
	}
}


ISR (INT1_vect){
	stepper_1.enabled = RUNNING_ON;	
	// need to preclear interrupts here
	EIMSK &= ~(1 << INT1);	// disable int1 interrupts
	EIMSK |= (1 << INT0);	// enable int0 interrupts
}


ISR (INT0_vect){
	stepper_1.enabled = RUNNING_OFF;	// disable the timers
	PORTF = 0;
	EIMSK &= ~(1 << INT0);  // disable the int0 interrupt
	EIMSK |= (1 << INT1);	// enable int1 interrupt
}

void port_init (void)
{
	DDRF = STEPPER_OUTPUT_PINS;
	DDRD = 0xF0;	// port 0 is interrupt pins
	PORTD = 3;	// initialize pull up resistors
	EICRA = (0x02 << ISC10);	// enable interrupts off of falling
	EIMSK = (0x01 << INT1);	// enable interrupts off of int0
	EIFR |= 1<<INTF1;
}

void set_rpm (struct stepper *stepper, unsigned int rpm){
	if (stepper->enabled == RUNNING_OFF){
		stepper->cmp = (STEPPER_DRIVE_FREQ/ STEPS_PER_ROT) * 60;
		stepper->cmp /= rpm;
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
	eeprom_rpm_high.data = char(rpm2write/100);
	eeprom_rpm_low.data = char (rpm2write - eeprom_rpm_high.data*100);
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


