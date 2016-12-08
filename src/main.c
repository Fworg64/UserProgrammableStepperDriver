/*
* Author: Marc Olberding
* Company: CTIPP @ NDSU
* Date: 12/5/16
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "LCD.h"
#include "usci.h"
#include "menu.h"
#include "timer.h"
#include "eeprom.h"

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

const char stepperarray[STEPPER_NUMBER_OF_STEPS]=
{
0x0C << BIT_PLACE, 0x04 << BIT_PLACE, 0x06 << BIT_PLACE, 0x02 << BIT_PLACE,
0x03 << BIT_PLACE, 0x01 << BIT_PLACE, 0x09 << BIT_PLACE, 0x08 << BIT_PLACE
};

//unsigned char direction = DIRECTION_UP;
//unsigned char running = RUNNING_ON;
//unsigned char numberofrots = 0;
unsigned char ms = 0;

void port_init (void);


struct stepper {
	unsigned long ctr;
	unsigned long cmp;
	unsigned char enabled;
	unsigned char direction;
	unsigned long number_of_rots;
	unsigned char index;
};

struct stepper stepper_1 = {.ctr = 0, .enabled = 0, .direction = DIRECTION_UP, .number_of_rots = 0, .index = 0};

void set_rpm (struct stepper *stepper, unsigned int rpm);

// portd0 is interrupt 0
//

int main (void)
{
	USART_init (103); // 9600 baud
	timer_init ();
	port_init ();
	lcd_init (USART_transmit_array);
	set_rpm (&stepper_1, 20);
	stepper_1.enabled = RUNNING_OFF;
	lcd_reset ();
	lcd_set_backlight (8);
	lcd_set_contrast(50);
	sei ();	// enable interrupts
	timer_start ();
	while (1)
	{
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



/*
These are definitions to be placed into TCCR1B
typedef enum timer_prescale_enum {
	STOPPED=0, PRESCALE_1 = 1, PRESCALE_8 = 2,
	PRESCALE_64 =3, PRESCALE_256=4,
	PRESCALE_1024 = 5, EXTERNAL_FALLING=6, EXTERNAL_RISING=7
}t_timer_prescale;

*/


