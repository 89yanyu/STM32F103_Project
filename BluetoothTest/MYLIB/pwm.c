/**************************************
* 本文件为电机控制相关函数
* 本文件对ENA,ENB,IN1,IN2,IN3,IN4联合控制
* 支持正反转，左右转
**************************************/
#include "pwm.h"
#include "stm32f10x_gpio.h"

//PWM初始化
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TIMInitStruct;
    TIM_OCInitTypeDef TIMOCInitStruct;

    //初始化时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    //初始化IO
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruct);

    //初始化定时器
    TIMInitStruct.TIM_Period = 7200;
    TIMInitStruct.TIM_Prescaler = 0;
    TIMInitStruct.TIM_ClockDivision = 0;
    TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIMInitStruct);

    //初始化PWM
    TIMOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIMOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIMOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIMOCInitStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM3, &TIMOCInitStruct);
    TIM_OC2Init(TIM3, &TIMOCInitStruct);

    TIM_Cmd(TIM3, ENABLE);

    //初始化IN1,IN2,IN3,IN4的IO口
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIOInitStruct);
}

void PWM_Change(short left, short right)
{
    //对左轮的正反转控制
    //如果大于零，正转
    //如果等于零，刹车
    //如果小于零，反转
    //如果需要把等于零时的刹车改为悬空，
    //只需将下面两个if的判断条件中的小于号改为小于等于
    if (left > 0)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_2);
    }
    else
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_2);
    }
    if (left < 0)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_4);
        left = -left;
    }
    else
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_4);
    }

    //对右轮的正反转控制
    //与左轮类似
    if (right > 0)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_3);
    }
    else
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_3);
    }
    if (right < 0)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_5);
        right = -right;
    }
    else
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_5);
    }
    //设置PWM
    TIM_SetCompare1(TIM3, left);
    TIM_SetCompare2(TIM3, right);
}
