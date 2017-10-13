#ifndef __89YY_EXINTERRUPT__

#define __89YY_EXINTERRUPT__

#include "stm32f10x.h"
#include "stm32f10x_exti.h"

typedef void(*ExInterrput_HandlerFunc)(void);

void ExInterrupt_Init(u8 GPIO_PortSource,
                      u8 GPIO_PinSource,
                      EXTITrigger_TypeDef EXTI_Trigger,
                      u8 NVIC_IRQChannelPreemptionPriority,
                      u8 NVIC_IRQChannelSubPriority);


void ExInterrupt_SetHandler(u8 GPIO_PortSource,
                            u8 GPIO_PinSource,
                            ExInterrput_HandlerFunc func);

#endif
