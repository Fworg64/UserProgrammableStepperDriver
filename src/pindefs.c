//Austin F. Oltmanns 12/6/2016
//implementation for pindef pin abstraction routines

#include <avr/io.h>

char scanmask = 0b01000000;
char shiftdir = 0; //1 is to the right
char scanmaskoffset =0;

void pindef_init()
{
	DDRA |= 0b01110000; //1 is output
	PORTA = 0b00000000; //disable pull up resistors, hadware has pull down for 4 rows
}

int readPins() //returns the pin pressed, -1 if no pin is pressed;
{
	PORTA = scanmask;
	scanmaskoffset = 3*(scanmask & 0b00100000 !=1) + 6*(scanmask & 0b00010000 !=1) + 9*(scanmask & 0b00001000 != 1);
	if (scanmask == 0b01000000 || scanmask == 0b00001000) shiftdir = !shiftdir;
	scanmask = (shiftdir ? scanmask>>1 : scanmask <<1);
		
	if (PINA & (1<<PA0)) return scanmaskoffset;
	if (PINA & (1<<PA1)) return scanmaskoffset +1;
	if (PINA & (1<<PA2)) return scanmaskoffset +2;


	/*
	PORTA= 0b01000000;
	if (PINA & (1<<PA3)) return 0;
	if (PINA & (1<<PA2)) return 1;
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
	*/
	return -1;
}
