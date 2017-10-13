#ifndef __89YY_DELAY__

#define __89YY_DELAY__

#include "stm32f10x.h"

void Delay_Init(void);

void Delay_ms(u32);

void Delay_us(u32);

void Delay_ms_Lazy(u32);

void Delay_us_Lazy(u32);

void Delay_ms_Dosth(u32, void(*)());

#endif

