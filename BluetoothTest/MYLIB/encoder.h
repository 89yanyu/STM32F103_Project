#ifndef __89YY_ENCODER__

#define __89YY_ENCODER__

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

void Encoder_Init_left(void);

int Encoder_GetSpeed_left(void);

void Encoder_Start_left(void);

void Encoder_Stop_left(void);

#endif
