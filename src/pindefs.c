//Austin F. Oltmanns 12/6/2016
//implementation for pindef pin abstraction routines

#include <avr/io.h>

void pindef_init()
{
	DDRA |= 0b01111000; //1 is output
	PORTA = 0b00000000; //disable pull up resistors, hadware has pull down for 4 rows
}

int readPins() //returns the pin pressed, -1 if no pin is pressed;
{

	PORTA= 0b01000000;
	if (PINA & (1<<PA2)) return 0;
	if (PINA & (1<<PA1)) return 1;
	if (PINA & (1<<PA0)) return 2;
	//need to debounce here
	PORTA=0;
	for (char godhelpus=0; godhelpus<100;godhelpus++);
	PORTA= 0b00100000;
	for (char godhelpus=0; godhelpus<100;godhelpus++);
	if (PINA & (1<<PA2)) return 3;
	if (PINA & (1<<PA1)) return 4;
	if (PINA & (1<<PA0)) return 5;
	//and here
	PORTA=0;
	for(char mayibeforgiven=0; mayibeforgiven<100;mayibeforgiven++);
    	PORTA= 0b00010000;
	for(char mayibeforgiven=0; mayibeforgiven<100;mayibeforgiven++);
	if (PINA & (1<<PA2)) return 6;
	if (PINA & (1<<PA1)) return 7;
	if (PINA & (1<<PA0)) return 8;
	//and here
	PORTA=0;
	for(char ididntmeanforittobethisway=0; ididntmeanforittobethisway<100;ididntmeanforittobethisway++);
	PORTA= 0b00001000;
	for(char ididntmeanforittobethisway=0; ididntmeanforittobethisway<100;ididntmeanforittobethisway++);
	if (PINA & (1<<PA2)) return 9;
	if (PINA & (1<<PA1)) return 10;
	if (PINA & (1<<PA0)) return 11;
	
	return -1;
}
