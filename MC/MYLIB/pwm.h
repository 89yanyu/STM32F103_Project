#ifndef __89YY_PWM__

#define __89YY_PWM__

#include "stm32f10x.h"

void PWM_Init(void);

#define PWM_Stop() PWM_Change(1500)

void PWM_Change(short p1);

#endif
