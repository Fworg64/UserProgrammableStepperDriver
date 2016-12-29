#include "timer.h"
#include "avr/io.h"

#define WGM_BITS			(0x01 << 3) //wgm12 (for ctc mode on ocr1a)

#define PRESCALE_BITS			0x001 //no prescale (1x)
#define CMP_VAL				5000




void timer_init (void){
	//PRR0 &= ~PRTIM1; 	// turn off power reduction timer in power reduction register //not for atmega 8515 you downt
	TCCR1A = 0b01000000; //toggle oc1a on compare match
	DDRD   |=0b00100000; //set oc1a output
	TCCR1B = WGM_BITS; 	// set timer mode to CTC //timer stopped, set prescale bits
	//TCCR1C = 0x00;
	OCR1A = CMP_VAL;	// set the compare value


	//now timer0, our ms timer
	TCCR0 = 0b00001011; //CTC mode, prescaler of 64
	OCR0 = 250; //250 *64 clocks = 16k clocks = 1 ms

	TIMSK = (1<<OCIE1A) | 1;  // enable timer1 interrupts based on compare with ocr1a and timer0 with ocr0
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
