//main.c
//Austin F. Oltmanns 12/6/2016
//Application code for user programmable stepper driver
 //#define __AVR_LIBC_DEPRECATED_ENABLE__ 1
#include <avr/io.h>
//#include <avr/interrupt.h>
#include "LCD.h"
#include "usci.h"
//#include "menu.h"
//#include "keypad.h"

int main()
{
	USART_init(103); //magic usart init number for 9600 baud
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (8);
	lcd_set_contrast(50);

	while (2)
	{
        USART_transmit('a');
	}
}
