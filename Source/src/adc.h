#ifndef __ADC_H
#define __ADC_H

#define ADC_DEMULT    0
#define ADC_DELITEL   3
#define ADC_LIGHT     4
#define ADC_IGNITION  7

extern unsigned int adc_value[8];

void InitADC (void);
void ADC_IRQ(void);

#endif // __ADC_H