#ifndef __89YY_ENCODER__

#define __89YY_ENCODER__

#include "stm32f10x.h"

void Encoder_Init(void);

int Encoder_GetSpeed(void);

void Encoder_Stop(void);

#endif
