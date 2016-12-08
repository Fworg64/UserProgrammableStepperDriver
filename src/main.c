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
#include "ms_timer.h"

#include "keypad.h"

#define STEPPER_NUMBER_OF_STEPS		8

#define BIT_PLACE			4

#define DIRECTION_UP			1
#define DIRECTION_DOWN			0

#define RUNNING_ON			1
#define RUNNING_OFF			0

#define STEPPER_OUTPUT_PINS		(0x0F << BIT_PLACE)

#define STEPS_PER_ROT			(STEPPER_NUMBER_OF_STEPS*50/4)

#define DEADBAND_MAX			25

// clock freq is 250KHz/2500

const char stepperarray[STEPPER_NUMBER_OF_STEPS]=
{
0x0C << BIT_PLACE, 0x04 << BIT_PLACE, 0x06 << BIT_PLACE, 0x02 << BIT_PLACE,
0x03 << BIT_PLACE, 0x01 << BIT_PLACE, 0x09 << BIT_PLACE, 0x08 << BIT_PLACE
};

unsigned char stepperctr;
unsigned char direction = DIRECTION_UP;
unsigned char running = RUNNING_ON;
unsigned char numberofrots;
unsigned char deadband = 0;
unsigned char ms;

void port_init (void);

//buffer for text stuff
char mycars[32] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
char tempindex=0;
char getanotherkey=1;

int main (void)
{
	struct eeprom_struct startup = {.startaddress = 0,.number_of_redundancy = 3, .data = RUNNING_OFF};
	struct eeprom_struct readme = {.startaddress = 0, .number_of_redundancy = 3};
	stepperctr = 0;
	numberofrots = 0;
	USART_init (103); // 9600 baud
	//eeprom_redundant_write (startup);
	ms_timer_init ();
	//timer_init ();
	//port_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (8);
	lcd_set_contrast(50);

	keypad_init();

	sei ();	// enable interrupts
	//timer_start ();
	ms_timer_start ();
	while (3)
	{
        if (getanotherkey) pollKeys(ms); //call this guy everyframe
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
        }
	}
	return 0;
}

ISR (TIMER3_COMPA_vect){
	if (ms++ >= MS_MAX){
		ms = 0;
	}
}



ISR (TIMER1_COMPA_vect)
{
	if (running == RUNNING_ON){
		PORTF = stepperarray[stepperctr];
		if (direction == DIRECTION_UP)
		{
			stepperctr++;
			numberofrots++;
			if (stepperctr >= STEPPER_NUMBER_OF_STEPS)
			{
				stepperctr = 0;
			}
		}
		else
		{
			if (stepperctr == 0)
			{
				stepperctr = STEPPER_NUMBER_OF_STEPS;
			}
			stepperctr--;
			numberofrots++;
		}
		if (numberofrots == STEPS_PER_ROT){
			running = RUNNING_OFF;
			numberofrots = 0;
			deadband = 0;
		}
	} else {
		if (deadband++ == DEADBAND_MAX){
			numberofrots = 0;
			running = RUNNING_ON;
		}
	}
}

void port_init (void)
{
	DDRF = STEPPER_OUTPUT_PINS;
}


/*
These are definitions to be placed into TCCR1B
typedef enum timer_prescale_enum {
	STOPPED=0, PRESCALE_1 = 1, PRESCALE_8 = 2,
	PRESCALE_64 =3, PRESCALE_256=4,
	PRESCALE_1024 = 5, EXTERNAL_FALLING=6, EXTERNAL_RISING=7
}t_timer_prescale;

*/




