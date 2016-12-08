#ifndef _USCI_H_
#define _USCI_H_

void USART_transmit (unsigned char);
void USART_init (unsigned int ubrr);//CALL TO INIT WITH 103 FOR 9600 BAUD
void USART_transmit_array (unsigned char*, unsigned char);

#endif
