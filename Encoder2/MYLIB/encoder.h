#ifndef __89YY_ENCODER__

#define __89YY_ENCODER__

#include "stm32f10x.h"

void Encoder_Init(void);

#define AllCount 390

int Encoder_GetSpeed(void);

#endif
