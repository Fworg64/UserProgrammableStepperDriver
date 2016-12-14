#include <avr/io.h>
#include <usci.h>

//#define BAUD_RATE(x) (F_CPU/(16*x) - 1)



void USART_transmit(char data)
{
   /* Wait for empty transmit buffer */
   while (!( UCSRA & (1<<UDRE)));
   /* Put data into buffer, sends the data */
   UDR = data;
}

void USART_init( unsigned int ubrr)
{
/*
Set baud rate
 */
	UBRRH = (unsigned char)((ubrr>>8)&0x0F);
	UBRRL = (unsigned char) ubrr;
/*
//more settings

Enable transmitter
 */
    UCSRA = 2;
	UCSRB =(1<<TXEN);
/*
Set frame format: 8data,1stop bit
 */
	UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);
}


void USART_transmit_array (char *data, unsigned char length)
{
	unsigned int ctr;
	for (ctr = 0; ctr<length; ctr++){
		USART_transmit (data[ctr]);
	}
}



// baud rate reg = fosc/16*baud -1 ||||| 16MHZ/(16*9600) -1;
// 2MHz/(16*9.6K) -1 = 12



// BIT 7: Read Complete
// to read if data is ready: check UCSR0A.UDRE0
/*  The communication format is 8 bit data, one
stop  bit,  no  parity  and  no  hand  shaking
.
*/


/*
* UCSR0B:
* BIT7: RXCIE0 Enables receiver interrupts
* BIT6: TXCIE0 Enables tx interrupts
* BIT5: UDRIE0 Enables isr upon empty data
* BIT4: RXEN0  Enables receiver, overwrites GPIO
* BIT3: TXEN0  Enables Transmitter, overwrites GPIO
* BIT2: UCSZ02 Character Size 0, sets tx/rx width
* BIT1: RXB80  Ninth data bit
* BIT0: TXB80  Ninth data bit
*/


/*
* UCSR0C:
*
*
*/
