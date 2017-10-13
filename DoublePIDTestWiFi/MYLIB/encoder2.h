#ifndef __89YY_ENCODER2__

#define __89YY_ENCODER2__

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

void Encoder2_Init(void);

#define AllCount 390

int Encoder2_GetSpeed(void);

void Encoder2_Start(void);

void Encoder2_Stop(void);

#endif
