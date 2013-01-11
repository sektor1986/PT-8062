#ifndef __SMC_H
#define __SMC_H

extern long smc_inp;
extern long smc_new;

void InitSMC(unsigned int x);
void ClearPosSMC(void);
void ZeroPosSMC(void);
void SmcParamsForReturn(void);
void SmcNormalParams(void);
void SMC_IRQ (void);

#endif // __SMC_H
