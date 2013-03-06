#include "mcu.h"
#include "icu.h"

//#define K                    4992
//#define KS                   K/10
//#define MAX_FREQUENSY        200000

#define SPEED_OUTPUT PDR04_P5

float MAX_FREQUENSY = 0;
unsigned long max_value;
unsigned long SPEED_OUTPUT_TIME = 0;
unsigned char PassCounter = 1;
unsigned char PASS_COUNTER_SPEED_IRQ = 1;

volatile unsigned int ICU1_old;
volatile unsigned int ICU1_new;
volatile unsigned int FRT0_ovl_cnt;
volatile unsigned int FRT0_ovl_cnt_new;
volatile unsigned int FRT0_ovl_cnt_old;
volatile unsigned long value;
unsigned char flag_freq = 0;

float frequency;

volatile unsigned long value_temp = 0;
volatile unsigned long W = 0;

void InitFRTimer0(void)
{  
	TCCS0  |=0x49;		//div  2, interrupt enabled
}

//------------------------------------------------------------------------------ 
// FRT0 (IRQ) 
//------------------------------------------------------------------------------ 
__interrupt void FRT0_IRQ (void)
{
	FRT0_ovl_cnt++;                      // Count the overflows 
	TCCS0_IVF = 0;
}

void InitExtInt0(void)
{
	PIER03_IE6 = 1;  // enable Port 03_6 input
	ELVR0_LALB00 = 0;	// Rising edge
	ELVR0_LALB01 = 1;	
	EIRR0_ER0 = 0;  // reset interrupt request 
	ENIR0_EN0 = 1;  // enable interrupt request 
}

__interrupt void EI0_IRQ (void)
{	
	if (EIRR0_ER0)
	{
	
	EIRR0_ER0 = 0;		// Сброс флага прерывания
	flag_freq = 1;

	if ((PassCounter == 0) && (ELVR0_LALB00 == 0))  //Если нужное прерывание (счетчик досчитан, импульс по фронту)
	{
		ICU1_new = TCDT0;                     // Save current ICU value
		
		FRT0_ovl_cnt_new = FRT0_ovl_cnt;      // Save current FRT value

		if (TCCS0_IVF && (TCDT0 < 0x8000))   // Check for FRT/ICU race condition 
		{                                     // In case that FRT was not yet handled 
			FRT0_ovl_cnt_new++;               // then increase temp FRT counter
		}
				
		// Calculation of time period
		value = abs(FRT0_ovl_cnt_new - FRT0_ovl_cnt_old) * 0x10000UL + ICU1_new - ICU1_old;

		ICU1_old = ICU1_new;                  // Save current ICU value as reference for next cycle
		FRT0_ovl_cnt_old = FRT0_ovl_cnt_new;  // Save current FRT value as reference for next cycle
  		
		PassCounter = PASS_COUNTER_SPEED_IRQ;
	}
	else
	{
		PassCounter--;
	}
	
	ELVR0_LALB00 = 0;

	} //end valid interrupt	
}
