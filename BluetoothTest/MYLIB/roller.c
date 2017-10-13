/**************************************
* ���ļ�Ϊ���ֿ�����غ���
* ���ڹ��ֽ���Ҫ��������
* ���Ա��ļ�֧�ֹ��ֵĿ��͹�
**************************************/
#include "stm32f10x_gpio.h"
#include "roller.h"

//���ֿ��Ƴ�ʼ��
void Roller_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    //��ʼ��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    //��ʼ��IO��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOE, GPIO_Pin_6);
}

//�ع���
void Roller_OFF(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_6);
}

//������
void Roller_ON(void)
{
    GPIO_SetBits(GPIOE, GPIO_Pin_6);
}

