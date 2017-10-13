#include "exinterrupt.h"
#include "stdlib.h"

#define MAX_CONFIG 4

u8 PortSource[MAX_CONFIG] = {GPIO_PortSourceGPIOA, GPIO_PortSourceGPIOE, GPIO_PortSourceGPIOE, GPIO_PortSourceGPIOE};
u8 PinSource[MAX_CONFIG] = {GPIO_PinSource0, GPIO_PinSource2, GPIO_PinSource3, GPIO_PinSource4};

u32 EXTI_Line[MAX_CONFIG] = {EXTI_Line0, EXTI_Line2, EXTI_Line3, EXTI_Line4};

u8 NVIC_IRQChannel[MAX_CONFIG] = {EXTI0_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn};

#define MAX_IRQn 5
#define BASE_IRQn EXTI0_IRQn

ExInterrput_HandlerFunc Func_IRQn[MAX_IRQn] = {NULL, NULL, NULL, NULL};

void ExInterrupt_Init(u8 GPIO_PortSource,
                      u8 GPIO_PinSource,
                      EXTITrigger_TypeDef EXTI_Trigger,
                      u8 NVIC_IRQChannelPreemptionPriority,
                      u8 NVIC_IRQChannelSubPriority)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    u8 Pos;
    static u8 first = 1;

    if (first)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        first = 0;
    }

    for (Pos = 0; Pos < MAX_CONFIG; Pos++)
    {
        if ((PinSource[Pos] == GPIO_PinSource) &&
            (PortSource[Pos] == GPIO_PortSource))
        {
            break;
        }
    }

    GPIO_EXTILineConfig(GPIO_PortSource, GPIO_PinSource);
    EXTI_InitStruct.EXTI_Line = EXTI_Line[Pos];
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = NVIC_IRQChannel[Pos];
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = NVIC_IRQChannelPreemptionPriority;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = NVIC_IRQChannelSubPriority;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void ExInterrupt_SetHandler(u8 GPIO_PortSource,
                            u8 GPIO_PinSource,
                            ExInterrput_HandlerFunc func)
{
    u8 Pos;
    for (Pos = 0; Pos < MAX_CONFIG; Pos++)
    {
        if ((PinSource[Pos] == GPIO_PinSource) &&
            (PortSource[Pos] == GPIO_PortSource))
        {
            break;
        }
    }

    Func_IRQn[NVIC_IRQChannel[Pos] - BASE_IRQn] = func;
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        Func_IRQn[EXTI0_IRQn - BASE_IRQn]();
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    return;
}
void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        Func_IRQn[EXTI2_IRQn - BASE_IRQn]();
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
    return;
}
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        Func_IRQn[EXTI3_IRQn - BASE_IRQn]();
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
    return;
}
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        Func_IRQn[EXTI4_IRQn - BASE_IRQn]();
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
    return;
}

