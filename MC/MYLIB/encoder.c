#include "encoder.h"

TIM_TypeDef *TIMER = TIM2;

void Encoder_Init()
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    TIM_ICInitTypeDef TimerICInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruct);

    TimerInitStruct.TIM_Period= 65535;
    TimerInitStruct.TIM_Prescaler = 0;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TimerInitStruct);

    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_BothEdge,
                               TIM_ICPolarity_BothEdge);

    TimerICInitStruct.TIM_Channel = TIM_Channel_1;
    TimerICInitStruct.TIM_ICFilter = 0;
    TimerICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TimerICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TimerICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInit(TIM2, &TimerICInitStruct);
    
    TimerICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM2, &TimerICInitStruct);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
}

void Encoder_Start(void)
{
    TIMER->CNT = 0;
    TIMER->CCR1 = 0;
    TIM_Cmd(TIM2, ENABLE);
}

void Encoder_Stop(void)
{
    TIM_Cmd(TIM2, DISABLE);
    TIMER->CNT = 0;
    TIMER->CCR1 = 0;
}

int Encoder_GetSpeed()
{
    int tmp = 0;
    if (TIMER->CR1 & TIM_CR1_CEN)
    {
        tmp = (short)TIM_GetCounter(TIMER);
        TIM_SetCounter(TIMER, 0);
    }
    return tmp;
}

