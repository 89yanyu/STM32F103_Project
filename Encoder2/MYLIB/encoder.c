#include "encoder.h"

TIM_TypeDef *TIMER = TIM4;

void Encoder_Init()
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    TIM_ICInitTypeDef TimerICInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOInitStruct);

    TimerInitStruct.TIM_Period= 65535;
    TimerInitStruct.TIM_Prescaler = 0;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TimerInitStruct);

    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_BothEdge,
                               TIM_ICPolarity_BothEdge);

    TimerICInitStruct.TIM_Channel = TIM_Channel_1;
    TimerICInitStruct.TIM_ICFilter = 6;
    TimerICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TimerICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TimerICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;

    TIM_ICInit(TIM4, &TimerICInitStruct);

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);//���TIM3�ĸ��±�־λ
    TIMER->CNT = 65535;

    TIM_Cmd(TIM4, ENABLE);   //����TIM4��ʱ��
}

int Encoder_GetSpeed()
{
    int tmp = TIMER->CNT;
    TIMER->CNT = 65535;
    TIM_Cmd(TIM4, ENABLE);   //����TIM4��ʱ��
    if (tmp > 32678) tmp = 65535 - tmp;
    else tmp = -tmp;
    return tmp;
}

