#include "pwm.h"
#include "stm32f10x_gpio.h"

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TIMInitStruct;
    TIM_OCInitTypeDef TIMOCInitStruct;

    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruct);

    TIMInitStruct.TIM_Period = 7200;
    TIMInitStruct.TIM_Prescaler = 0;
    TIMInitStruct.TIM_ClockDivision = 0;
    TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIMInitStruct);

    TIMOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIMOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIMOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIMOCInitStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM3, &TIMOCInitStruct);
    TIM_OC2Init(TIM3, &TIMOCInitStruct);

    TIM_Cmd(TIM3, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIOInitStruct);
}

void PWM_Change(short p1, short p2)
{
    // 2 4
    if (p1 > 0)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_2);
        GPIO_ResetBits(GPIOE, GPIO_Pin_4);
    }
    else if (p1 < 0)
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_2);
        GPIO_SetBits(GPIOE, GPIO_Pin_4);
    }
    
    if (p2 > 0)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_3);
        GPIO_ResetBits(GPIOE, GPIO_Pin_5);
    }
    else if (p2 < 0)
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_3);
        GPIO_SetBits(GPIOE, GPIO_Pin_5);
    }
    TIM_SetCompare1(TIM3, p1);
    TIM_SetCompare2(TIM3, p2);
}
