#include "mcu.h"
#include "uart.h"

unsigned char RXptr = 0;
unsigned char Rxbuf[MAXBUF];


void InitUsart0(void)
{	
	PDR13_P4 = 0;
	DDR13_D4 = 0;
	PIER13_IE4 = 1; // Enable SIN0 Port Pin
	// Initialize UART asynchronous mode
  
	// BGR0 = 832;  //  9600 Baud @  8MHz
	// BGR0 = 416;  // 19200 Baud @  8MHz
	// BGR0 = 207;  // 38400 Baud @  8MHz
  
	//UART0_BGR0 = 1666; //  9600 Baud @ 16MHz
	// BGR0 = 832;  // 19200 Baud @ 16MHz
	// BGR0 = 416;  // 38400 Baud @ 16MHz

	// BGR0 = 2083; //  9600 Baud @ 20MHz
	// BGR0 = 1041; // 19200 Baud @ 20MHz
	// BGR0 = 520;  // 38400 Baud @ 20MHz

	// BGR0 = 2499; //  9600 Baud @ 24MHz
	// BGR0 = 1249; // 19200 Baud @ 24MHz
	// BGR0 = 624;  // 38400 Baud @ 24MHz

	 UART0_BGR0 = 3332; //  9600 Baud @ 32MHz
	// BGR0 = 1666; // 19200 Baud @ 32MHz
	// BGR0 = 832;  // 38400 Baud @ 32MHz 
	UART0_SSR0 = 0x02;             // LSB first 
	UART0_SMR0 = 0x0D;             // enable SOT0, reset, Asynchronous normal mode 
	UART0_SCR0 = 0x17;            // 8N1, clear possible errors, enable RX, TX 
}

/*
void Uart0_SendByte(char ch)   // sends a char
{
  while (UART0_SSR0_TDRE == 0);      // wait for transmit buffer empty
  UART0_TDR0 = ch;                   // put ch into buffer
}
*/


// Reception Interrupt Service 
__interrupt void RX_USART0(void)
{
	if ((UART0_SSR0 & 0xE0) != 0)     // Check for errors PE, ORE, FRE
	{
		UART0_SCR0_CRE = 1;            // Clear error flags
	}
	if (RXptr < MAXBUF)   // Only, if End of Buffer not reached 
	{
		Rxbuf[RXptr] = UART0_RDR0;   // Fill Reception Buffer
		RXptr++;    // Update Buffer Pointer
	}
	
	
}

