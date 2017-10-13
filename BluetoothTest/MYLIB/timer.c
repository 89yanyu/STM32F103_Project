/**************************************
* 本文件为定时器相关函数
* 本文件仅完成定时器的设置，定时中断函数由用户完成
**************************************/
#include "timer.h"
#include "stm32f10x_tim.h"

Timer_HandlerFunc Handler = NULL;

void Timer_Init()
{
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    NVIC_InitTypeDef NVICInitStruct;

    //初始化时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    //初始化定时器
    TimerInitStruct.TIM_Period= 100;
    TimerInitStruct.TIM_Prescaler = 7199;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TimerInitStruct);

    //打开定时器中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    //初始化中断
    NVICInitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVICInitStruct.NVIC_IRQChannelSubPriority = 3;
    NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICInitStruct);

    //默认开启
    TIM_Cmd(TIM2, ENABLE);

}

//设置自定义中断处理函数
void Timer_SetHandler(Timer_HandlerFunc func)
{
    Handler = func;
}

//定时器中断
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        //如果当前没有处理函数，则什么都不做
        if (Handler != NULL)
        {
            Handler();
        }
    }
}
