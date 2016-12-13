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
	/*
	DDRB |= ~0b10110000;
	DDRC |= ~0b01000000;
	DDRD |= ~0b11001011;
	DDRE |= ~0b01000000;
	DDRF |= ~0b00000011; //changed pins from 5,6 to 0,1 for arduino micro

	//ENable pull up resistors
	PORTB |= 0b10110000;
	PORTC |= 0b01000000;
	PORTD |= 0b11001011;
	PORTE |= 0b01000000;
	PORTF |= 0b00000011;
	*/

	DDRA |= 0b01110000; //1 is output
	PORTA = 0b00000000; //disable pull up resistors, hadware has pull down for 4 rows
}

int readPins() //returns the pin pressed, -1 if no pin is pressed;
{
	/*
	switch (pinnumber)
	{
		case 0:
		return ((PIND & (1<<PD6)) == 0);
		case 1:
		return ((PINB & (1<<PB7)) == 0);
		case 2:
		return ((PINB & (1<<PB5)) == 0);
		case 3:
		return ((PINB & (1<<PB4)) == 0);
		case 4:
		return ((PINE & (1<<PE6)) ==0);
		case 5:
		return ((PIND & (1<<PD7)) ==0);
		case 6:
		return ((PINC & (1<<PC6)) == 0);
		case 7:
		return ((PIND & (1<<PD4)) == 0);
		case 8:
		return ((PIND & (1<<PD0)) == 0);
		case 9:
		return ((PIND & (1<<PD1)) == 0);
		case 10:
		return ((PINF & (1<<PF0)) == 0);
		case 11:
		return ((PINF & (1<<PF1)) ==0);
	}
	*/
	PORTA= 0b01000000;
	if (PINA & (1<<PA3)) return 1;
	if (PINA & (1<<PA2)) return 2;
	if (PINA & (1<<PA1)) return 3;
	if (PINA & (1<<PA0)) return 4;
	PORTA= 0b00100000;
	if (PINA & (1<<PA3)) return 5;
	if (PINA & (1<<PA2)) return 6;
	if (PINA & (1<<PA1)) return 7;
	if (PINA & (1<<PA0)) return 8;
    PORTA= 0b00010000;
    if (PINA & (1<<PA3)) return 9;
	if (PINA & (1<<PA2)) return 10;
	if (PINA & (1<<PA1)) return 11;
	if (PINA & (1<<PA0)) return 12;
	return -1;
}
