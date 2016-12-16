#ifndef __EEPROM_H_
#define __EEPROM_H_


#define EEPROM_IS_CORRUPT	1
#define EEPROM_IS_NO_ERR	0


struct eeprom_struct {
	unsigned int startaddress;
	unsigned char data;
	unsigned char number_of_redundancy;
	unsigned char err;
};


struct eeprom_16_bit {
	struct eeprom_struct high_byte;
	struct eeprom_struct low_byte;
	unsigned int data;
	unsigned char err;
};

void eeprom_redundant_read (struct eeprom_struct *);
void eeprom_redundant_write (struct eeprom_struct);
void eeprom_16_bit_write (struct eeprom_16_bit *, int to_write);
void eeprom_16_bit_read (struct eeprom_16_bit *);
void eeprom_16_bit_init (struct eeprom_16_bit * to_setup, unsigned int start_address, unsigned int number_of_redundancy);

#endif
