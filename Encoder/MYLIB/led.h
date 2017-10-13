#ifndef __89YY_LED__

#define __89YY_LED__

#include "stm32f10x.h"

void LED_Init(void);

#define LED_0   0x1
#define LED_1   0x2
#define LED_ALL 0x3

void LED_ON(u8);

void LED_OFF(u8);

void LED_Change(u8);

#endif
