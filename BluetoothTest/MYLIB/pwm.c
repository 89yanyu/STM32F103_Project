/**************************************
* ���ļ�Ϊ���������غ���
* ���ļ���ENA,ENB,IN1,IN2,IN3,IN4���Ͽ���
* ֧������ת������ת
**************************************/
#include "pwm.h"
#include "stm32f10x_gpio.h"

//PWM��ʼ��
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TIMInitStruct;
    TIM_OCInitTypeDef TIMOCInitStruct;

    //��ʼ��ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    //��ʼ��IO
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruct);

    //��ʼ����ʱ��
    TIMInitStruct.TIM_Period = 7200;
    TIMInitStruct.TIM_Prescaler = 0;
    TIMInitStruct.TIM_ClockDivision = 0;
    TIMInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIMInitStruct);

    //��ʼ��PWM
    TIMOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIMOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIMOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIMOCInitStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM3, &TIMOCInitStruct);
    TIM_OC2Init(TIM3, &TIMOCInitStruct);

    TIM_Cmd(TIM3, ENABLE);

    //��ʼ��IN1,IN2,IN3,IN4��IO��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIOInitStruct);
}

void PWM_Change(short left, short right)
{
    //�����ֵ�����ת����
    //��������㣬��ת
    //��������㣬ɲ��
    //���С���㣬��ת
    //�����Ҫ�ѵ�����ʱ��ɲ����Ϊ���գ�
    //ֻ�轫��������if���ж������е�С�ںŸ�ΪС�ڵ���
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

    //�����ֵ�����ת����
    //����������
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
    //����PWM
    TIM_SetCompare1(TIM3, left);
    TIM_SetCompare2(TIM3, right);
}
