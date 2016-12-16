/*
* Author: Marc Olberding
* Company: CTIPP @ NDSU
* Project: Stepper Driver
* File: LCD.c
* Purpose: To Provide methods for commanding the NKC1602SK-NSW-BBW-33V33
*/

#include "LCD.h"
#include "cursordefs.h"

#define MAX_CURSOR_POS	16
#define SEGMENT_LENGTH	8

#define SEGMENT_1_POS		0
#define SEGMENT_2_POS		0x08
#define SEGMENT_3_POS		0x40
#define SEGMENT_4_POS		0x48

#define NUMBER_OF_SEGMENTS	4

static void (*send_data)(char *, unsigned char);
static char workingbuffer[MAX_LCD_STRING_LENGTH];
const unsigned char segment_pos[NUMBER_OF_SEGMENTS] = {SEGMENT_1_POS, SEGMENT_2_POS, SEGMENT_3_POS, SEGMENT_4_POS};

int get_string_len (char *buffer);

/*
* init_LCD: sets up the lcd driver
* params:
* 	a pointer to a function which sends data over UART
*/

void lcd_set_cursor (unsigned char cursorpos)
{
	workingbuffer[0] = LCD_CMD_PREFIX;
	workingbuffer[1] = LCD_SET_CURSOR_POS;
	workingbuffer[2] = cursorpos;
	send_data (workingbuffer, LCD_SET_CURSOR_POS_LENGTH);
}
void lcd_init (void (*send)(char *, unsigned char))
{
	send_data = send;
}

/*
* lcd_reset: clears the screen and resets the cursor
* params:
*	nothing
*/

void lcd_reset ()
{
	workingbuffer[0] = LCD_CMD_PREFIX;
	workingbuffer[1] = LCD_CLEAR_SCREEN;
	send_data (workingbuffer, LCD_CLEAR_SCREEN_LENGTH);
}

/*
* write_segment:
* param:
*	takes a buffer, cuts off anything past SEGMENT_LENGTH bytes
*	takes the segment number, indexed to 0.
*/


int get_string_len (char *string)
{
	unsigned char ctr = 0;
	while (string[ctr++] != '\0');
	return ctr;
}



void lcd_send_string (char *string)
{
	unsigned int len = get_string_len (string) - 1;
	lcd_reset ();	// set the cursor to the home position
	if (len > 0)	// if there is anything to send, send it
	{
		send_data (string, len);			// send the string, including the null character
		if (len > MAX_CURSOR_POS)						// if the length is enough that it should span two lines
		{	
			lcd_set_cursor (segment_pos[2]);	// set the cursor to the 2nd line
			send_data (string+MAX_CURSOR_POS, len - MAX_CURSOR_POS);	// send the second half of the string
		}
	}
	
}

/*
* lcd_set_contrast: sets the contrast level for the LCD
* params:
* 	level: contrast level, default is 40 the contrast increases with the number
*/


void lcd_set_contrast (unsigned char level)
{
	if (level >= LCD_DISPLAY_CONTRAST_MIN)
	{
		if (level <= LCD_DISPLAY_CONTRAST_MAX)
		{
			workingbuffer[0] = LCD_CMD_PREFIX;
			workingbuffer[1] = LCD_SET_DISPLAY_CONTRAST;
			workingbuffer[2] = level;
			send_data(workingbuffer, LCD_SET_DISPLAY_CONTRAST_LENGTH);
		}
	}
}


/*
* lcd_set_backlight: sets the backlight level
* params:
* 	unsigned char backlight: the level of the backlight. Increases with the number, default of 5
*/

void lcd_set_backlight (unsigned char backlight)
{
	if (backlight >= LCD_BACKLIGHT_MIN)
	{
		if (backlight <= LCD_BACKLIGHT_MAX)
		{
			workingbuffer[0] = LCD_CMD_PREFIX;
			workingbuffer[1] = LCD_SET_BACKLIGHT;
			workingbuffer[2] = backlight;
			send_data(workingbuffer, LCD_SET_BACKLIGHT_LENGTH);
		}
	}
}

/*
* lcd_set_baud: sets the baud rate for RS232 communication
*		default baud rate is 9600
* params:
*	new_baud_rate: an enumerated type, defined in LCD.h
*/

void lcd_set_baud (e_lcd_baud_rate new_baud_rate)
{
	workingbuffer[0] = LCD_CMD_PREFIX;
	workingbuffer[1] = LCD_SET_BAUD_RATE;
	workingbuffer[2] = new_baud_rate;
	send_data (workingbuffer, LCD_SET_BAUD_RATE_LENGTH);
}

/*
* lcd_display_baud: tells the LCD to display the current baud rate
* No parameters
*/

void lcd_display_baud (void)
{
	workingbuffer[0] = LCD_CMD_PREFIX;
	workingbuffer[1] = LCD_DISPLAY_BAUD_RATE;
	send_data (workingbuffer, LCD_DISPLAY_BAUD_RATE_LENGTH);
}

void lcd_turn_on_blinking_cursor (void)
{
	workingbuffer[0] = LCD_CMD_PREFIX;
	workingbuffer[1] = LCD_TURN_ON_BLINKING_CURSOR;
	send_data (workingbuffer, LCD_TURN_ON_BLINKING_CURSOR_LENGTH);

}

void lcd_turn_off_blinking_cursor (void)
{
	workingbuffer[0] = LCD_CMD_PREFIX;
	workingbuffer[1] = LCD_TURN_OFF_BLINKING_CURSOR;
	send_data (workingbuffer, LCD_TURN_OFF_BLINKING_CURSOR_LENGTH);
}
