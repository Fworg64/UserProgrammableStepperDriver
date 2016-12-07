

#include "eeprom.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void eeprom_write(unsigned char data, unsigned int address)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE));
	/* Set up address and Data Registers */
	EEAR = address;
	EEDR = data;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

unsigned char eeprom_read(unsigned int address)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE));
	/* Set up address register */
	EEAR = address;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}


int eeprom_redundant_read (struct eeprom_struct *eeprom_data){
	unsigned char tmp = SREG & 0x80;
	if (tmp){
		cli ();
	}
	unsigned char ctr;
	unsigned char lastdata = eeprom_read (eeprom_data->startaddress);
	for (ctr = 1; ctr < eeprom_data->number_of_redundancy; ctr++){
		eeprom_data->data = eeprom_read (eeprom_data->startaddress + ctr);
		if (eeprom_data->data != lastdata){
			return (EEPROM_IS_CORRUPT);
		}
		lastdata = eeprom_data->data;
	}
	if (tmp){
		sei ();
	}
	return (EEPROM_IS_NO_ERR);
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

