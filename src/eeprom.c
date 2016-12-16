

#include "eeprom.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include "eeprom.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void eeprom_write(unsigned char data, unsigned int address)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address and Data Registers */
	EEAR = address;
	EEDR = data;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMWE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEWE);
}

unsigned char eeprom_read(unsigned int address)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address register */
	EEAR = address;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}


void eeprom_redundant_read (struct eeprom_struct *eeprom_data){
	unsigned char tmp = SREG & 0x80;
	if (tmp){
		cli ();
	}
	unsigned char ctr;
	unsigned char lastdata = eeprom_read (eeprom_data->startaddress);
	for (ctr = 1; ctr < eeprom_data->number_of_redundancy; ctr++){
		eeprom_data->data = eeprom_read (eeprom_data->startaddress + ctr);
		if (eeprom_data->data != lastdata){
			eeprom_data->err = EEPROM_IS_CORRUPT;
		}
		lastdata = eeprom_data->data;
	}
	if (tmp){
		sei ();
	}
}

void eeprom_redundant_write (struct eeprom_struct eeprom_data){
	unsigned char ctr;
	unsigned char tmp = SREG & 0x80;
	if (tmp){
		cli ();
	}
	for (ctr = 0; ctr < eeprom_data.number_of_redundancy; ctr++){
		eeprom_write (eeprom_data.data, eeprom_data.startaddress + ctr);
	}
	if (tmp){
		sei ();
	}
}


void eeprom_16_bit_write(struct eeprom_16_bit *writeto, int to_write)
{
	unsigned int tmp;
	writeto->high_byte.data = (char) (to_write/100);
	tmp = writeto->high_byte.data * 100;
	tmp = to_write - tmp;
	writeto->low_byte.data =(char) (tmp);
	eeprom_redundant_write(writeto->high_byte);
	eeprom_redundant_write(writeto->low_byte);
	writeto->low_byte.err = 0;
	writeto->high_byte.err = 0;
	writeto->data = to_write;
	writeto->err = 0;
}

void eeprom_16_bit_read (struct eeprom_16_bit *to_read)
{
	eeprom_redundant_read (&(to_read->high_byte));
	eeprom_redundant_read (&(to_read->low_byte));
	to_read->err = to_read->high_byte.err;
	to_read->err += to_read->low_byte.err;
	to_read->data = to_read->high_byte.data * 100;
	to_read->data += to_read->low_byte.data;
}

void eeprom_init (struct eeprom_struct *to_init, unsigned int start_address, unsigned int number_of_redundancy)
{
	to_init->startaddress = start_address;
	to_init->err = EEPROM_IS_NO_ERR;
	to_init->number_of_redundancy = number_of_redundancy;
	to_init->data = 0x00;
}

void eeprom_16_bit_init (struct eeprom_16_bit *to_init, unsigned int start_address, unsigned number_of_redundancy){
	to_init->low_byte.startaddress = start_address;
	to_init->low_byte.err = EEPROM_IS_NO_ERR;
	to_init->low_byte.number_of_redundancy = number_of_redundancy;
	to_init->high_byte.startaddress = start_address + number_of_redundancy;
	to_init->high_byte.err = EEPROM_IS_NO_ERR;
	to_init->high_byte.number_of_redundancy	= number_of_redundancy;
}
