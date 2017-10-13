#ifndef __89YY_ENCODER2__

#define __89YY_ENCODER2__

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

void Encoder_Init_right(void);

int Encoder_GetSpeed_right(void);

void Encoder_Start_right(void);

void Encoder_Stop_right(void);

#endif
