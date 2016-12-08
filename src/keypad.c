//Keypad.c
//Austin F. Oltmanns 12/6/2016
//Implementation for keypad.h

#include "pindefs.h"

int keypressed =-1; //-1 means null/no key pressed
char keys[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'};


int keypad_init()
{
	pindef_init();
	return 0;
}

void pollKeys()
{
	//read each bit
	for (int u=0;u<12;u++)
	{
		if (readPin(u))
		{
			keypressed = u;
		}
	}
	return;
}

char getKey()
{
	return (keypressed ==-1) ? '\0' : keys[keypressed];
}

void clearKey()
{
	keypressed=-1;
}
