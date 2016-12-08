//Austin F. Oltmanns 12/6/2016
//implementation for pindef pin abstraction routines

#include <avr/io.h>

void pindef_init()
{
    /* Arduiono Micro
    portd 0,1,4,7,6
    portb 4,5,7
    portc 6
    porte 6
    portf 5, 6
    */
	//set direction of pins to input
	DDRB |= ~0b10110000;
	DDRC |= ~0b01000000;
	DDRD |= ~0b11001011;
	DDRE |= ~0b01000000;
	DDRF |= ~0b01100000;

	//ENable pull up resistors
	PORTB |= ~0b10110000;
	PORTC |= ~0b01000000;
	PORTD |= ~0b11001011;
	PORTE |= ~0b01000000;
	PORTF |= ~0b01100000;
}

int readPin(int pinnumber)
{
	switch (pinnumber)
	{
		case 0:
		return !(PORTD & (1<<6));
		case 1:
		return !(PORTB & (1<<7));
		case 2:
		return !(PORTB & (1<<5));
		case 3:
		return !(PORTB & (1<<4));
		case 4:
		return !(PORTE & (1<<6));
		case 5:
		return !(PORTD & (1<<7));
		case 6:
		return !(PORTC & (1<<6));
		case 7:
		return !(PORTD & (1<<4));
		case 8:
		return !(PORTD & (1<<0));
		case 9:
		return !(PORTD & (1<<1));
		case 10:
		return !(PORTF & (1<<5));
		case 11:
		return !(PORTF & (1<<6));
	}
	return 0;

}
