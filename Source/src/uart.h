#ifndef __UART_H
#define __UART_H

#define MAXBUF     8
extern unsigned char RXptr;
extern unsigned char Rxbuf[MAXBUF];

void InitUsart0(void);
__interrupt void RX_USART0(void);

#endif // __UART_H
