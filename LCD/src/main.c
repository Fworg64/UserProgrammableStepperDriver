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

#define STEPPER_NUMBER_OF_STEPS		8

#define CMP_VAL				0xFFFF
#define BIT_PLACE			4

#define DIRECTION_UP			1
#define DIRECTION_DOWN			0

#define RUNNING_ON			1
#define RUNNING_OFF			0

#define OUTPUT_PINS			(0x0F << BIT_PLACE)
#define WGM_BITS			(0x01 << 3)

#define PRESCALE_BITS			0x005

const char stepperarray[STEPPER_NUMBER_OF_STEPS]=
{
0x0C << BIT_PLACE, 0x04 << BIT_PLACE, 0x06 << BIT_PLACE, 0x02 << BIT_PLACE,
0x03 << BIT_PLACE, 0x01 << BIT_PLACE, 0x09 << BIT_PLACE, 0x08 << BIT_PLACE
};

unsigned char stepperctr;
unsigned char direction = DIRECTION_UP;
unsigned char startup;
unsigned char running = RUNNING_ON;
unsigned int numberofrots;


void timer_init (void);
void port_init (void);
void timer_start (void);
void timer_stop (void);

int main (void)
{
	stepperctr = 0;
	numberofrots = 0;
	unsigned char working[8] = "working";
	unsigned char tmp;
	unsigned char lasttmp = 0;
	USART_init (103); // 9600 baud
	timer_init ();
	port_init ();
	lcd_init (USART_transmit_array);
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
	PORTD = stepperarray[stepperctr];
	if (direction == DIRECTION_UP)
	{
		stepperctr++;
		if (stepperctr >= STEPPER_NUMBER_OF_STEPS)
		{
			stepperctr = 0;
			numberofrots++;
		}
	} 
	else 
	{
		if (stepperctr == 0)
		{
			stepperctr = STEPPER_NUMBER_OF_STEPS;
			numberofrots++;
		} 
		stepperctr--;
	}
}

void timer_start (void)
{
	TCCR1B |= PRESCALE_BITS;
}

void timer_stop (void)
{
	TCCR1B &= ~PRESCALE_BITS;
}

void timer_set_comp (unsigned int cmp)
{
	timer_stop ();	// stop the timer
	OCR1A = cmp;
}

void port_init (void)
{
	DDRD = OUTPUT_PINS;
	PORTD = OUTPUT_PINS;
}


/*
These are definitions to be placed into TCCR1B
typedef enum timer_prescale_enum {
	STOPPED=0, PRESCALE_1 = 1, PRESCALE_8 = 2,
	PRESCALE_64 =3, PRESCALE_256=4,
	PRESCALE_1024 = 5, EXTERNAL_FALLING=6, EXTERNAL_RISING=7
}t_timer_prescale;

*/


void timer_init (void){
	PRR0 &= ~PRTIM1; 	// turn off power reduction timer in power reduction register	
	TCCR1A = 0x00;
	TCCR1B = WGM_BITS; 	// timer stopped, set prescale bits
	TCCR1C = 0x00; 	
	OCR1A = CMP_VAL;	// set the compare value
	TIMSK1 = (1<<OCIE1A); // enable interrupts based on compare
}


