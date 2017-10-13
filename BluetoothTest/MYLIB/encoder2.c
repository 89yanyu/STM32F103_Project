/**************************************
* 本文件为编码器相关函数
* 本文件适用于AB相编码器
* 本文件为encoder.c的翻版，不过使用的是TIM5
* 注释详见encoder.c
**************************************/
#include "encoder2.h"

TIM_TypeDef *TIMER2 = TIM5;

void Encoder_Init_right()
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    TIM_ICInitTypeDef TimerICInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruct);

    TimerInitStruct.TIM_Period= 65535;
    TimerInitStruct.TIM_Prescaler = 0;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMER2, &TimerInitStruct);

    TIM_EncoderInterfaceConfig(TIMER2, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_BothEdge,
                               TIM_ICPolarity_BothEdge);

    TimerICInitStruct.TIM_Channel = TIM_Channel_1;
    TimerICInitStruct.TIM_ICFilter = 0;
    TimerICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TimerICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TimerICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInit(TIMER2, &TimerICInitStruct);

    TimerICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIMER2, &TimerICInitStruct);

    TIM_ClearFlag(TIMER2, TIM_FLAG_Update);

}

void Encoder_Start_right(void)
{
    TIM_SetCounter(TIMER2, 0);
    TIM_Cmd(TIMER2, ENABLE);
}

void Encoder_Stop_right(void)
{
    TIM_Cmd(TIMER2, DISABLE);
    TIM_SetCounter(TIMER2, 0);
}

int Encoder_GetSpeed_right()
{
    int tmp = -1;
    if (TIMER2->CR1 & TIM_CR1_CEN)
    {
        tmp = 0;
        tmp = (short)TIM_GetCounter(TIMER2);
        TIM_SetCounter(TIMER2, 0);
    }
    return tmp;
}

