/**************************************
* 本文件为编码器相关函数
* 本文件适用于AB相编码器
**************************************/
#include "encoder.h"

TIM_TypeDef *TIMER = TIM4;

//编码器初始化
void Encoder_Init_left()
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    TIM_ICInitTypeDef TimerICInitStruct;

    //初始化时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    //初始化IO口
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOInitStruct);

    //初始化定时器
    //注意，Period不可以改为0，原因未知，可能与计数方向有关
    TimerInitStruct.TIM_Period= 65535;
    TimerInitStruct.TIM_Prescaler = 0;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMER, &TimerInitStruct);

    //初始化定时器编码器模式
    TIM_EncoderInterfaceConfig(TIMER, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_BothEdge,
                               TIM_ICPolarity_BothEdge);

    TimerICInitStruct.TIM_Channel = TIM_Channel_1;
    TimerICInitStruct.TIM_ICFilter = 0;
    TimerICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TimerICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TimerICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInit(TIMER, &TimerICInitStruct);

    TimerICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIMER, &TimerICInitStruct);

    //清除定时器更新中断
    TIM_ClearFlag(TIMER, TIM_FLAG_Update);

}

//开启编码器捕获
void Encoder_Start_left(void)
{
    TIM_SetCounter(TIMER, 0);
    TIM_Cmd(TIMER, ENABLE);
}

//关闭编码器捕获
void Encoder_Stop_left(void)
{
    TIM_Cmd(TIMER, DISABLE);
    TIM_SetCounter(TIMER, 0);
}

//获取编码器捕获值
int Encoder_GetSpeed_left()
{
    int tmp = 0;
    if (TIMER->CR1 & TIM_CR1_CEN)
    {
        //获取并清空，否则会累加
        tmp = (short)TIM_GetCounter(TIMER);
        TIM_SetCounter(TIMER, 0);
    }
    return tmp;
}

