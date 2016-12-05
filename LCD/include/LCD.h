#ifndef _LCD_H_
#define _LCD_H_

/* Author: Marc Olberding
* Company: CTIPP
* Project: Stepper Drivers
* File: LCD.h
* Purpose: Provides an interface to interacting with the NKC1602SK-NSW-BBW-33V33 LCD board. It is assumed that it is interacting via RS232. This must be supplied with a function that sends serial data
*/

#define MAX_LCD_STRING_LENGTH	32

/* 
baud rate definitions:
1 - 300
2 - 1200
3 - 2400
4 - 9600
5 - 14400
6 - 19.2K
7 - 57.6K
8 - 115.2K
*/

typedef enum lcd_baud_enum {
BAUD_300 = 1, BAUD_1200 = 2, BAUD_2400 = 3,
BAUD_9600 = 4, BAUD_14400 = 5, BAUD_19200 = 6, 
BAUD_57600 = 7, BAUD_115200 = 8} e_lcd_baud_rate;

void lcd_init (void (*send)(unsigned char *, unsigned char)); // send this a pointer to a function that sends serial data
void lcd_reset (void);
void lcd_set_cursor (unsigned char cursorpos);
void lcd_set_contrast (unsigned char level);
void lcd_set_backlight (unsigned char backlight);
void lcd_set_baud (e_lcd_baud_rate new_baud_rate);
void lcd_display_baud (void);
void lcd_write_segment (unsigned char *buffer, unsigned char segmentnumber);
void lcd_send_string (unsigned char *string); //MUST BE NULL TERMINATED OR UNIVERSE WILL IM(EX)PLODE
void lcd_turn_on_blinking_cursor (void);
void lcd_turn_off_blinking_cursor (void);

#endif
