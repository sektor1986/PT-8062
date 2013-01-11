#ifndef __ICU_H
#define __ICU_H

extern volatile unsigned long value;
extern unsigned char flag_freq;
extern unsigned long max_value;
extern volatile unsigned long value_calc_koef;
extern float frequency;
extern float MAX_FREQUENSY;
extern double km;
extern double km_sut;
extern float ProbegForSave;
extern unsigned long SPEED_OUTPUT_TIME;
extern unsigned char PASS_COUNTER_SPEED_IRQ;

extern unsigned long K;
extern volatile unsigned long W;
extern float MetrInImp;
extern float ProbegM;
extern unsigned char acceleration_limit;
extern unsigned char flag_calc_koef;

void InitFRTimer0(void);
void InitICU1(void);
void InitExtInt0(void);
void UpdateSpeetOutput();

#endif // __ICU_H