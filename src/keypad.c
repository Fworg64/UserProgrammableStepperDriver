//Keypad.c
//Austin F. Oltmanns 12/6/2016
//Implementation for keypad.h

#include "pindefs.h"
#define DEBOUNCETIME 25


int keypressed =-1; //-1 means null/no key pressed
int lastkeypressed =-1;
char keys[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'};
unsigned char nextms =DEBOUNCETIME;
unsigned char lastms =0;

int keypad_init()
{
	pindef_init();
	return 0;
}

void pollKeys(unsigned char ms)
{
    if (ms >=255-2*DEBOUNCETIME && ms <= 255-DEBOUNCETIME) //ms is in a bad spot (region 2) and nextms will be close to 255
    {
        if (ms >=nextms )
        {
            lastkeypressed = keypressed;
            keypressed = readPins();
            nextms =0; //set next ms to 0, code below will ensure that the next key will be polled only after a reacharound
            lastms =ms;
        }
    }
    else //ms is in region 1 or 3
    {
        if (ms>nextms && (lastms<255-2*DEBOUNCETIME || lastms> 255-DEBOUNCETIME)) //if ms>nextms && lastms was NOT in region 2
        {                                                                         //i.e, nextms will be in region 1 or 2
            lastkeypressed = keypressed;
            keypressed = readPins();
            nextms =ms+25;
            lastms =ms;
        }
        else if (lastms >=255-2*DEBOUNCETIME && lastms <= 255-DEBOUNCETIME) //if lastms was in region 2
        {
            if (ms < 255-2*DEBOUNCETIME) //and ms is in region 1
            {
                lastkeypressed = keypressed;
                keypressed = readPins();
                nextms =ms+25;
                lastms =ms;
            }
            //do nothing if ms is between 255 and 255-debouncetime
        }
    }
	return;
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

int wasKeyPressed()
{
    return (keypressed != -1);
}

int wasKeyReleased()
{
    return (lastkeypressed != keypressed);
}

void clearKey()
{
	keypressed=-1;
	lastkeypressed =-1;
	return;
}
