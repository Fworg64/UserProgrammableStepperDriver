#ifndef __EEPROM_H_
#define __EEPROM_H_


#define EEPROM_IS_CORRUPT	1
#define EEPROM_IS_NO_ERR	0


struct eeprom_struct {
	unsigned int startaddress;
	unsigned char data;
	unsigned char number_of_redundancy;
};


struct eeprom_16_bit {
	struct eeprom_struct high_byte;
	struct eeprom_struct low_byte;
};


int eeprom_redundant_read (struct eeprom_struct*);
void eeprom_redundant_write (struct eeprom_struct);




#endif
