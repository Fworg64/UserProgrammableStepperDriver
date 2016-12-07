#include "ms_timer.h"
#include "avr/io.h"

#define WGM_BITS			(0x01 << 3)

#define PRESCALE_BITS			0x003
#define CMP_VAL				250

void ms_timer_init (void){
	PRR0 &= ~PRTIM3; 	// turn off power reduction timer in power reduction register	
	TCCR3A = 0x00;
	TCCR3B = WGM_BITS; 	// timer stopped, set prescale bits
	TCCR3C = 0x00; 	
	OCR3A = CMP_VAL;	// set the compare value
	TIMSK3 = (1<<OCIE3A); // enable interrupts based on compare
}


void ms_timer_start (void)
{
	TCCR3B |= PRESCALE_BITS;
}

void ms_timer_stop (void)
{
	TCCR3B &= ~PRESCALE_BITS;
}

void ms_timer_set_comp (unsigned int cmp)
{
	ms_timer_stop ();	// stop the timer
	OCR3A = cmp;
}
