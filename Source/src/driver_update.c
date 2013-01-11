#include "mcu.h"
#include "basics.h"
#include "button.h"
#include "driver_update.h"
#include "timer.h"
#include "icu.h"
#include "lcd.h"
#include "smc.h"
#include "utils.h"
#include "flash.h"
#include "adc.h"
#include "rtc.h"
#include "j1939.h"
#include "backlight.h"
#include "uart.h"

#define VERS         11                // Версия программы 1.1 26.12.2012

#define DEMULT    PDR08_P1 
#define DELITEL   PDR08_P2

#define MAX_TAH 3000

const float koef[12] = {2.08, 2.34, 2.4, 2.6, 2.9, 3.0, 3.2, 3.227, 3.32, 3.7, 4.0, 4.075};

static en_gui_mode_t    m_enGuiMode = Standart;

unsigned int PA_8141 = 0;
unsigned char count_rewriteK = 0;
unsigned char visible_countRewrK = 0;
unsigned char Time_visible_countRewrK = 0;

unsigned char Num = 0;                          // номер редактируемого символа
unsigned char blink = 0;                        // мерцание
unsigned int TimerBlink = 0;                    // Переменная таймера мерцания 
unsigned long value_fr_old = 0;                 // Переменная для хренения предыдущего значения счетчика частоты
unsigned int TimerFr = 0;                       // Переменная таймера обнуленя частоты
unsigned int TIME_OBNUL_FR = 0;                 // 
unsigned long probeg = 0;
unsigned long probegSut_old = 0;     
unsigned char flag_smcUpdate = 1;
unsigned int nul_to_spd = 0; 
unsigned char timer_show_probeg_in_mode_empty = 0;  

float RPM = 0.0;

unsigned char save_params_flag = 0;


float I = 2.08;
unsigned char NUM_K = 0;

unsigned int timer_mode_empty = 0;

void Timer_1s(void);
void Timer_60s(void);
void EditTopLine(void);
void EditBottomLine(void);
void Probeg(void);
void UpdateTime(void);
void EditTime(void);
void RecalcVariables(void);
void RecalcStepSpeedLimit(void);
void SaveKFrom_Uart(void);
void InitMode(en_gui_mode_t mode);

// Функции сохранения параметров
unsigned char Save_K(unsigned int koef);
unsigned int Load_K(void);
unsigned char SaveProbeg(unsigned long probeg);
unsigned long LoadProbeg(void);
unsigned char SaveProbegSut(unsigned long probeg);
unsigned long LoadProbegSut(void);
unsigned char SavePass(unsigned int pass);
unsigned int LoadPass(void);
void LoadAllParams(void);
unsigned int SaveAllParams(void);
unsigned int SaveAllParamsSecondary(void);


//***************Изменения********************
// 14.11.2011 - Добавлена возможность просматривать количество изменений K
// в режиме установки времени установить время 23:54, в часах останется 23,
// в минутах в течении 5 секунд будет коичество изменений К

void DriverInit(void)
{	

	DDR08_D1 = 1;	// Demultiplikator
	DDR08_D2 = 1;	// Delitel
	
	DDR08_D3 = 0;	// Input Fr
	
	LoadAllParams();		// Чтение параметров из flash
	RecalcVariables();
	Timer_Start(TIMER_ID_MAIN, 20, TRUE, UpdateSMCLCD);	
		
}

void RecalcVariables(void)
{
	I = koef[NUM_K];
	
	TIME_OBNUL_FR = (unsigned long)50; 
	MAX_FREQUENSY = ((float)MAX_TAH*I/ 10.0);
	PASS_COUNTER_SPEED_IRQ = 2;
	
}

void UpdateSMC(void)
{
	unsigned long temp_sms_inp = 0;
	float TempSpeed = 0.0;
	
	if (TimerFr > TIME_OBNUL_FR)
	{
		if (flag_freq == 0)
		{
			value = 0;
		}
		flag_freq = 0;
		TimerFr = 0;
	}
	else
	{
		TimerFr++;
	}
		
	if (value != 0)
		frequency = 16000000.0/((float)value/(float)(PASS_COUNTER_SPEED_IRQ+1));
	else
		frequency = 0; 
	if (m_enGuiMode == Standart)
		RPM = frequency*10.0 / (float)I;
	else
		RPM = 250 + 250 * NUM_K;
				
	// Шкала 220 гр. 1 гр. -> 512 шагов 	
	// 1об/мин = 220/3000 = 0,0733 гр
	// 1об/мин = 0,0733 * 512 = 37.545	
	

	temp_sms_inp = RPM * 37.545;

	if (temp_sms_inp > 125000)
		temp_sms_inp = 125000;
	if (temp_sms_inp == 0)
		nul_to_spd++;
	smc_inp = temp_sms_inp;
}

void UpdateSMCLCD(void)
{	
	unsigned long new_backlight = 0;
//	unsigned char flag_smcUpdate = 0;
	

	// Уровень подсветки
	//Backlight = adc_value[ADC_LIGHT]  >> 3;
	if (adc_value[ADC_LIGHT] > 200)
	{ 
		new_backlight = ((adc_value[ADC_LIGHT] << 5) / adc_value[ADC_IGNITION]) << 2; 	
		if (new_backlight > 250)	
			new_backlight = 250;
		//new_backlight = ( (float)adc_value[ADC_LIGHT] / (float)(MAX(adc_value[ADC_IGNITION], adc_value[ADC_BAT])) ) * 250.0;
	}
	else 
		new_backlight = 0;
	
	if (adc_value[ADC_DEMULT] < 200)
		DEMULT = 1;
	else
		DEMULT = 0;

	if (adc_value[ADC_DELITEL] < 200)
		DELITEL = 1;
	else
		DELITEL = 0;

//	if (adc_value[ADC_BAT] > 200)
//	{
		if (new_backlight > Backlight)
			Backlight++;
		else if (new_backlight < Backlight)
			Backlight--;		
//	}
//	else	
//		Backlight = Backlight >> 1;
		
	if ((adc_value[ADC_IGNITION] > 338) && (adc_value[ADC_IGNITION] < 710))
	{	
		if ((adc_value[ADC_IGNITION] > 346) && (adc_value[ADC_IGNITION] < 691))
		{
			flag_smcUpdate = 1;
			timer_mode_empty = 0;
		}
		save_params_flag = 1;
	}
	else
	{
		if (timer_mode_empty < 10)
			timer_mode_empty++;
		else
		{
			SmcParamsForReturn();
			smc_inp = smc_inp >> 1;
			flag_smcUpdate = 0;
			RPM = 0.0;		
		}	
	}
	
	if (flag_smcUpdate)
		UpdateSMC();			
}

void ButoonPress(unsigned char enState)
{	
	static uint8_t firstPress = 1;
	
	if (firstPress)
	{
		firstPress = 0;
		return;
	}
	if (enState == 1)	           //short press
	{
		switch (m_enGuiMode)
		{
			case SetK:
				if (NUM_K < 11)
					NUM_K++;
				else
					NUM_K = 0;
				break;
				
			default:
				break;
		}		
	}
	 
	if (enState == 2)     //long press
	{
		switch (m_enGuiMode)
		{
			case SetK:
				RecalcVariables();
				SaveAllParams();			
				m_enGuiMode = Standart;
				break;
				
			default:
				break;
		}			
	}
}

void SetModePass(void)
{
	m_enGuiMode = SetK;	
}

void LoadAllParams(void)
{
	unsigned char data[2];
	unsigned char ReadOk = 0;
	unsigned int temp = 0;
	unsigned char i = 0;
	
	for (i = 0; i < 1; i++)
	{
		temp = *(unsigned int __far*)(SA2+i*2);	
		data[i*2] = (temp >> 8) & 0xFF;
		data[i*2+1] = temp & 0xFF;		
	}	
	
	// Если контрольная сумма совпадает 
	if (data[0] == data[1])
	{
		ReadOk = 1;
	}
	else
	{
		// Если ошибка чтения то считываем дублирающие данные
		for (i = 0; i < 1; i++)
		{
			temp = *(unsigned int __far*)(SA3+i*2);	
			data[i*2] = (temp >> 8) & 0xFF;
			data[i*2+1] = temp & 0xFF;		
		} 
		ReadOk= 0;
	}
	
			
	//K = Load_K();
	NUM_K = data[0];
	if (NUM_K > 11)
		NUM_K = 0;

	if (ReadOk)
		//Сохраняем значения
		SaveAllParamsSecondary();
	Data_flash_SectorErase(SA2);
}

unsigned int SaveAllParams(void)
{
	unsigned int result = 0;
	unsigned char data[2];
	unsigned char CRC = 0;
	unsigned int temp = 0;
	unsigned char i = 0;
	
	// Сохранение K
	data[0] = NUM_K;
	data[1] =  NUM_K;
	
//	Data_flash_SectorErase(SA2);					
	for (i = 0; i < 1; i++)
	{
		temp = ((unsigned int)data[2*i] << 8) | (unsigned int)data[2*i+1];
		Data_Flash_write(SA2+i*2, temp);
		while (temp != *(unsigned int __far*)(SA2+i*2));	
	}
		
	return result;
}

unsigned int SaveAllParamsSecondary(void)
{
	unsigned int result = 0;
	unsigned char data[2];
	unsigned char CRC = 0;
	unsigned int temp = 0;
	unsigned char i = 0;
	
	// Сохранение K
	data[0] = NUM_K;
	data[1] =  NUM_K;
	

	Data_flash_SectorErase(SA3);					
	for (i = 0; i < 1; i++)
	{
		temp = ((unsigned int)data[2*i] << 8) | (unsigned int)data[2*i+1];
		Data_Flash_write(SA3+i*2, temp);
		while (temp != *(unsigned int __far*)(SA3+i*2));	
	}
		
	return result;
}
