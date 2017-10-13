#include "timer.h"
#include "stm32f10x_tim.h"

Timer_HandlerFunc Handler = NULL;

void Timer_Init()
{
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    NVIC_InitTypeDef NVICInitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TimerInitStruct.TIM_Period= 100;
    TimerInitStruct.TIM_Prescaler = 7199;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TimerInitStruct);

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    NVICInitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVICInitStruct.NVIC_IRQChannelSubPriority = 3;
    NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICInitStruct);

    TIM_Cmd(TIM1, ENABLE);

}

void Timer_SetHandler(Timer_HandlerFunc func)
{
    Handler = func;
}

void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        if (Handler != NULL)
        {
            Handler();
        }
    }
}
