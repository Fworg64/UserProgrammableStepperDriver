// Austin F. Oltmanns 12/6/2016
//Code for interfacing with a keypad that brings a pin to GND for each key pressed
#ifndef _KEYPAD_H_
#define _KEYPAD_H_

int keypad_init();
void pollKeys(); //call this funcion once per frame
char getKey(); //returns the last key pressed, null if buffer is clear
int wasKeyPressed(); //returns 1 if a key was just pressed;
int wasKeyReleased(); //returns true if a key was released
void clearKey(); //call this to clear the key buffer



#endif
