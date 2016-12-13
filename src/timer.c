#include "timer.h"
#include "avr/io.h"

#define WGM_BITS			(0x01 << 3)

#define PRESCALE_BITS			0x001
#define CMP_VAL				160




void timer_init (void){
	PRR0 &= ~PRTIM1; 	// turn off power reduction timer in power reduction register	
	TCCR1A = 0x00;
	TCCR1B = WGM_BITS; 	// timer stopped, set prescale bits
	TCCR1C = 0x00; 	
	OCR1A = CMP_VAL;	// set the compare value
	TIMSK1 = (1<<OCIE1A); // enable interrupts based on compare
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
