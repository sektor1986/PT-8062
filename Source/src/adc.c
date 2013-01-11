#include "mcu.h"
#include "adc.h"

unsigned int adc_value[8];

void InitReloadTimer1 (void)
{
//  TMR1 = 65535;  // set reload value (65535+1) x 1us  => 65 ms
  TMR1 = 5000;
  TMCSR1 = 0x81B;  // prescaler 2us at 32 MHz
}

void InitADC (void)
{	
//  ADCSL = 0x80;   // single, 10 bit, trigger by Software and Reload Timer 1
//  ADCSH = 0xA8;

  ADCSL = 0xC0;   // stop mode, 10 bit
  ADCSH = 0xA0;
  
  ADTGCRH0_RLTE = 0;
  
  ADSR = 0x9130;   // 24 cy. sampling; 88 cy. conversion; channel 9-16 
  ADER1_ADE9 = 1;
  ADER1_ADE12 = 1;
  ADER1_ADE13 = 1;
  ADER2_ADE16 = 1;
  
  ADER2_ADE21 = 0;
  ADER2_ADE23 = 0;
	
  ADCSH_STRT = 1;        // start adc
//  InitReloadTimer1();
}


//__interrupt void ADC_IRQ(void)
void ADC_IRQ(void)
{
	static unsigned char AdcCnt = 0;
//	ADCSL = 0x80;   // single, 10 bit, trigger by Software and Reload Timer 1
//	ADCSH = 0xA8;
	ADCSL = 0xC0;   // Clear INT
	ADCSH = 0xA0;
  
	adc_value[AdcCnt] = ADCRLH;  			// write to array
	AdcCnt++;
	if (AdcCnt > 7)
	AdcCnt = 0;
	ADCSH_STRT = 1;		          // re-start ADC
}

