/**************************************
* ���ļ�Ϊ��������غ���
* ���ļ�������AB�������
**************************************/
#include "encoder.h"

TIM_TypeDef *TIMER = TIM4;

//��������ʼ��
void Encoder_Init_left()
{
    GPIO_InitTypeDef GPIOInitStruct;
    TIM_TimeBaseInitTypeDef TimerInitStruct;
    TIM_ICInitTypeDef TimerICInitStruct;

    //��ʼ��ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    //��ʼ��IO��
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOInitStruct);

    //��ʼ����ʱ��
    //ע�⣬Period�����Ը�Ϊ0��ԭ��δ֪����������������й�
    TimerInitStruct.TIM_Period= 65535;
    TimerInitStruct.TIM_Prescaler = 0;
    TimerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TimerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMER, &TimerInitStruct);

    //��ʼ����ʱ��������ģʽ
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

    //�����ʱ�������ж�
    TIM_ClearFlag(TIMER, TIM_FLAG_Update);

}

//��������������
void Encoder_Start_left(void)
{
    TIM_SetCounter(TIMER, 0);
    TIM_Cmd(TIMER, ENABLE);
}

//�رձ���������
void Encoder_Stop_left(void)
{
    TIM_Cmd(TIMER, DISABLE);
    TIM_SetCounter(TIMER, 0);
}

//��ȡ����������ֵ
int Encoder_GetSpeed_left()
{
    int tmp = 0;
    if (TIMER->CR1 & TIM_CR1_CEN)
    {
        //��ȡ����գ�������ۼ�
        tmp = (short)TIM_GetCounter(TIMER);
        TIM_SetCounter(TIMER, 0);
    }
    return tmp;
}

