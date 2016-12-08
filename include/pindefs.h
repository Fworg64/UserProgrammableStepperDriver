//Austin F. Oltmanns 12/6/2016
//pindef file for keypad.c
//provides hardware abstraction for keypad module
//must define first 12 pins and only 12 pins that keypad is plugged into
#ifndef _PINDEFS_H_
#define _PINDEFS_H_

//#include <avr/io.h>

void pindef_init();
int readPin(int pinaddress);

#endif
