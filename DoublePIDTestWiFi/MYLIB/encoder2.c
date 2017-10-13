#include "encoder.h"

TIM_TypeDef *TIMER2 = TIM5;

void Encoder2_Init()
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

    TIM_ClearFlag(TIMER2, TIM_FLAG_Update);//清除TIM3的更新标志位

    //TIM_Cmd(TIM4, ENABLE);   //启动TIM4定时器
}

void Encoder2_Start(void)
{
    TIMER2->CNT = 0;
    TIMER2->CCR1 = 0;
    TIM_Cmd(TIMER2, ENABLE);
}

void Encoder2_Stop(void)
{
    TIM_Cmd(TIMER2, DISABLE);
    TIMER2->CNT = 0;
    TIMER2->CCR1 = 0;
}

int Encoder2_GetSpeed()
{
    int tmp = 0;
    if (TIMER2->CR1 & TIM_CR1_CEN)
    {
        tmp = (short)TIM_GetCapture1(TIMER2);
    }
    TIMER2->CNT = 0;
    TIMER2->CCR1 = 0;
    TIM_Cmd(TIMER2, ENABLE);   //启动TIM4定时器
    return tmp;
}

