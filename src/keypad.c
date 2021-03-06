//Keypad.c
//Austin F. Oltmanns 12/6/2016
//Implementation for keypad.h

#include "pindefs.h"
#define NUMKEYS 12

int keypressed =-1; //-1 means null/no key pressed
int lastkeypressed =-1;
char keys[] = {'#', '0', '7', '9', '8', '4', '6', '5', '1', '3', '2', '*'};
char keyreleased =0; //bool for keyrelease

int keypad_init()
{
	pindef_init();
	return 0;
}

void pollKeys()
{
        lastkeypressed = keypressed;
        keypressed = readPins();
	if (lastkeypressed != keypressed) keyreleased =1;
}

char getKey()
{
	//return (keypressed ==-1) ? '\0' : keys[keypressed];
	if(keypressed == -1){
        return '\0';
	}else {
        return keys[keypressed];
	}
}

int isKeyPressed()
{
    return (keypressed != -1);
}

int wasKeyReleased()
{
    return (keyreleased);
}

void clearKey()
{
	keypressed=-1;
	lastkeypressed =-1;
	keyreleased=0;
	return;
}
