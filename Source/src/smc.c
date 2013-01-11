#include "mcu.h"
#include "options.h"
#include "smc.h"
#include "icu.h"
#include "backlight.h"

  #define PWC  PWC1_PWC1
  #define PWEC PWC1_PWEC1
  #define PWCA PWC1_PWC11
  #define PWSA PWC1_PWS11
  #define PWCB PWC1_PWC21
  #define PWSB PWC1_PWS21

#define TMRLR2  TMR2

unsigned int time_update_smc = 0;
unsigned int time_update_speed_out = 0;
  
  
/*----------- Global Variables --------------------------------------*/
long smc_pt1, smc_clc1;   
long smc_inp, smc_new, smc_old, smc_velo, smc_velo_old; 
 
int  smc_dn,  smc_vmax, smc_amax; 

//volatile unsigned char dir;     // direction

/*---- sin\cos Lookup table for microstepping------------------------*/

unsigned char const SMC_TAB_CS[128]={0,  3,  6,  9, 13, 16, 19, 22,     
                                     25, 28, 31, 34, 37, 41, 44, 47,
                                     50, 53, 56, 59, 62, 65, 68, 71,
                                     74, 77, 80, 83, 86, 89, 92, 95,
                                     98,100,103,106,109,112,115,117,
                                     120,123,126,128,131,134,136,139,
                                     142,144,147,149,152,154,157,159,
                                     162,164,167,169,171,174,176,178,
                                     180,183,185,187,189,191,193,195,
                                     197,199,201,203,205,207,208,210,
                                     212,214,215,217,219,220,222,223,
                                     225,226,228,229,231,232,233,234,
                                     236,237,238,239,240,241,242,243,
                                     244,245,246,247,247,248,249,249,
                                     250,251,251,252,252,253,253,253,
                                     254,254,254,255,255,255,255,255 };

/*-------------------------------------------------------------------*/

/* Lookup tables for quadrant management*/

unsigned char const smc_quad_a[4]={0x02, 0x10, 0x10, 0x02};
unsigned char const smc_quad_b[4]={0x50, 0x50, 0x42, 0x42};
unsigned char const smc_quad_b1[4]={0x42, 0x42, 0x50, 0x50};


void InitSMC(unsigned int x) /* set up all neccesary CPU resources  */ 
{
  /* initialise cpu output port for the motor */ 
	DDR08=0x0F;                   /* assign pins as output      */ 
	PHDR08 = 0x0F; 
	PWEC = 0x03; 
    /* initialise low pass filters     */ 
	smc_inp=0;                    /* clear target position    */ 
	smc_pt1=0;                    /* clear actual position         */ 
	smc_new=0; smc_old=0;         /* clear actual outputs        */ 
    /* initialise variables for physical limits */ 
	smc_dn =   6;                 /* set up damping grade       */ 
	smc_vmax = 100;               /* set up velocity limit          */ 
	smc_amax =  1;                /* set up acceleration limit      */ 
    /* initialise reload timer 2   
    time_update_smc             */ 
	TMRLR2  = x;                /* set reload value in [us] 1      */ 
	TMCSR2  = 0x81B;            /* prescaler 1us at 32MHz         */ 
}

void ZeroPosSMC(void)
{
  	smc_inp = -125000;	
}

void ClearPosSMC(void)
{
	smc_inp=0;                    /* clear target position    */ 
	smc_pt1=0;                    /* clear actual position         */ 
	smc_new=0; smc_old=0;         /* clear actual outputs        */ 
	smc_dn =   8;                 /* set up damping grade       */ 
	smc_amax =  1;                /* set up acceleration limit      */ 
	smc_vmax = 100;               /* set up velocity limit          */ 
}

void SmcParamsForReturn(void)
{
	smc_dn =   6;                 /* set up damping grade       */ 
	smc_vmax = 200;               /* set up velocity limit          */ 
	smc_amax =  4;                /* set up acceleration limit      */ 	
}

void SmcNormalParams(void)
{
	smc_dn =   8;                 /* set up damping grade       */ 
	smc_amax =  1;                /* set up acceleration limit      */ 
	smc_vmax = 100;               /* set up velocity limit          */ 	
}

void smc_out(long ustp) 
{ 
	int q,d,smc_a,smc_b;    /* some squeeze intermediate memories  */ 
	
	q=((ustp>>8) & 3);      /* normalise the over all granulation  to 1024 microsteps per polpair change  */ 
	d=((ustp>>1) & 127);    /* normalise the inner granulation to 512 microsteps per polpair change so that the Bit0 of ustp is don't care! */ 
	
	smc_a=SMC_TAB_CS[d];    /* preload of sin component      */ 
	smc_b=SMC_TAB_CS[127-d];/* preload of cos component note the trick with the enlarged table, which can be used in reverse order  */ 
	if ((q & 1)==1) 
	{                       /* decide where to go whatever    */ 
		PWCA=smc_a;        /* set up the sin value for coil A    */ 
		PWCB=smc_b;        /* set up the cos value for coil B    */ 
	}
	else 
	{                       /* otherwise change the signs    */ 
		PWCA=smc_b;        /* set up the cos value for coil A    */ 
		PWCB=smc_a;        /* set up the sin value for coil B    */ 
	} 
	//PWC=0xE8;              /* startover with the resource operation  */ 
	PWC=0xB8;
	PWSA=smc_quad_a[q];    /* arming the signal for coil A    */ 
	#if (SMC_TYPE == SMC_TYPE_VID29_02)
		PWSB=smc_quad_b[q];    /* arming the signal for coil B    */ 
	#else
		PWSB=smc_quad_b1[q]; 
	#endif
}

void smc__lpf(void) 
{ /* this tiny calculation should be done in a less part of a millisecond  */  
	
	smc_old = smc_new;     /* yesterdays future is passed today */ 
    smc_velo_old = smc_velo;

	smc_clc1 = (smc_pt1 << smc_dn) - smc_pt1;
	smc_pt1 = (smc_clc1 + smc_inp) >> smc_dn;
	
	smc_clc1 = (smc_new << smc_dn) - smc_new;
	smc_new = (smc_clc1 + smc_pt1) >> smc_dn;

		
	smc_velo = smc_new - smc_old;
	smc_clc1 = smc_velo - smc_velo_old;
	
	if (smc_clc1 >= 0)
	{
		if (smc_clc1 > smc_amax)
		{
			smc_velo = smc_velo_old + smc_amax;
		}
	}
	else
	{
		if (smc_clc1 < (-smc_amax))
		{
			smc_velo = smc_velo_old - smc_amax;
		}		
	}
	
	if (smc_velo >= 0)
	{
		if (smc_velo > smc_vmax)
		{
			smc_velo = smc_vmax;
		}
	}
	else
	{
		if (smc_velo < (-smc_vmax))
		{
			smc_velo = -smc_vmax;
		}	
	}
	
	smc_new = smc_old + smc_velo;
	
}

//__interrupt void SMC_IRQ (void)
void SMC_IRQ (void)
{   /* background task for motor controlling */ 
	time_update_smc++;
	time_update_speed_out++;
	if (time_update_smc == 25)   // 1000 mks = 25*40
	{
		smc_out(smc_new);  /*  force the output first, for less delay glitch  */ 
		smc__lpf();       /* calculate next cycle output value     */  
		time_update_smc = 0;

	}
	if (time_update_speed_out == 2) //80 mks = 2*40
	{
		time_update_speed_out = 0;
	}
	
	UpdateBacklight();			//Подсветка
	TMCSR2_UF = 0;    /* reset underflow interrupt request flag    */ 
}
