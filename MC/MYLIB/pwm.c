#include "pwm.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include "usart.h"

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TIMInitStruct;
    TIM_OCInitTypeDef TIMOCInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruct);

    TIMInitStruct.TIM_Period = 20000;
    TIMInitStruct.TIM_Prescaler = 72;
    TIMInitStruct.TIM_ClockDivision = 0;
    TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIMInitStruct);

    TIMOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIMOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIMOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIMOCInitStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM3, &TIMOCInitStruct);

    TIM_Cmd(TIM3, ENABLE);
    TIM_SetCompare1(TIM3, 1500);
}

void PWM_Change(short p1)
{
    p1 = 1500 - p1;
    if (p1 > 2200) p1 = 2200;
    if (p1 < 800) p1 = 800;
    TIM_SetCompare1(TIM3, p1);
}
