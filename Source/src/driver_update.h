#ifndef __DRIVER_UPDATE_H
#define __DRIVER_UPDATE_H

typedef enum en_gui_mode
{
    Standart,
    SetK
} en_gui_mode_t;

void DriverInit(void);
void ButoonPress(unsigned char enState);
void SetModePass(void);
void UpdateSMCLCD(void);



#endif