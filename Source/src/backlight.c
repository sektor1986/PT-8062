#include "mcu.h"
#include "backlight.h"
#include "timer.h"
#include "adc.h"

unsigned long Backlight = 0; //0..250

#define STATE_BACKLIGHT PDR05_P0


void UpdateBacklight(void);

void InitBacklight(void)
{
	DDR05_D0 = 1;
	STATE_BACKLIGHT = 0;
}

void UpdateBacklight(void)
{
	static unsigned int count = 0;
	
	//Backlight = adc_value[ADC_LIGHT] >> 2;
/*	if (adc_value[ADC_LIGHT] < 50)
	{
		Backlight = 0;
		count = 1;
	}
*/	Backlight &= 0xFF;
	if (count > Backlight)
		STATE_BACKLIGHT = 0;
	
	if (count == 255)
	{
		count = 0;
		if (Backlight != 0)
			STATE_BACKLIGHT = 1;
	}
	count++;
}
