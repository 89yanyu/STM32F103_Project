#ifndef __89YY_TIMER__

#define __89YY_TIMER__

#include "stm32f10x.h"
#include <stdlib.h>

typedef void(*Timer_HandlerFunc)(void);

void Timer_Init(void);
void Timer_SetHandler(Timer_HandlerFunc);

#endif
