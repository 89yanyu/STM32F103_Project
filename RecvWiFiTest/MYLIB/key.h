#ifndef __89YY_KEY__

#define __89YY_KEY__

#include "stm32f10x.h"

/* 4-0: KEY0 KEY1 KEY2 WKUP */
#define KEY_0 0x8
#define KEY_1 0x4
#define KEY_2 0x2
#define WK_UP 0x1

#define GPIO_Source_WK_UP GPIO_PortSourceGPIOA, GPIO_PinSource0
#define GPIO_WK_UP GPIOA, GPIO_Pin_0

#define GPIO_Source_KEY_2 GPIO_PortSourceGPIOE, GPIO_PinSource2
#define GPIO_KEY_2 GPIOE, GPIO_Pin_2

#define GPIO_Source_KEY_1 GPIO_PortSourceGPIOE, GPIO_PinSource3
#define GPIO_KEY_1 GPIOE, GPIO_Pin_3

#define GPIO_Source_KEY_0 GPIO_PortSourceGPIOE, GPIO_PinSource4
#define GPIO_KEY_0 GPIOE, GPIO_Pin_4

void Key_Init(void);

u8 KEY_GetState(void);

#endif

