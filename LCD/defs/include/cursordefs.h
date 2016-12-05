#ifndef _CURSOR_DEFS_H_
#define _CURSOR_DEFS_H_

/*
* Author: Marc Olberding
* Company: CTIPP @ NDSU
* File: cursordefs.h
* Purpose: To provide constant definitions for interfacing with the NKC1602SK-NSW-BBW-33V33 LCD module
*
*/


#define LCD_CMD_PREFIX			0xFE

#define LCD_SET_BAUD_RATE		0x61
#define LCD_SET_BAUD_RATE_LENGTH	3

/*
1 - 300
2 - 1200
3 - 2400
4 - 9600
5 - 14400
6 - 19.2K
7 - 57.6K
8 - 115.2K
*/

// requires 20 microseconds for baud rate change to take place

#define LCD_SET_I2C_ADDRESS		0x62
#define LCD_SET_I2C_ADDRESS_LENGTH	3

#define LCD_RESET_CURSOR		(0x46)
#define LCD_RESET_CURSOR_LENGTH		2

#define LCD_CLEAR_SCREEN		0x51
#define LCD_CLEAR_SCREEN_LENGTH		2

#define LCD_SET_DISPLAY_CONTRAST	(0x52)
#define LCD_SET_DISPLAY_CONTRAST_LENGTH	3
// value is 1 through 50
#define LCD_DISPLAY_CONTRAST_MIN	1
#define LCD_DISPLAY_CONTRAST_MAX	50

#define LCD_SET_BACKLIGHT		0x53
#define LCD_SET_BACKLIGHT_LENGTH	3
// value is 1 through 8
#define LCD_BACKLIGHT_MIN		1
#define LCD_BACKLIGHT_MAX		8

#define LCD_DISPLAY_BAUD_RATE		0x71
#define LCD_DISPLAY_BAUD_RATE_LENGTH	2

#define LCD_DEFAULT_I2C_ADDRESS		0x32

#define LCD_SET_CURSOR_POS		0x45
#define LCD_SET_CURSOR_POS_LENGTH	3

#define LCD_TURN_ON_BLINKING_CURSOR	0x4B
#define LCD_TURN_ON_BLINKING_CURSOR_LENGTH	2

#define LCD_TURN_OFF_BLINKING_CURSOR	0x4C	
#define LCD_TURN_OFF_BLINKING_CURSOR_LENGTH	2


#endif
