/**************************************
* 本文件为滚轮控制相关函数
* 由于滚轮仅需要正向满速
* 所以本文件支持滚轮的开和关
**************************************/
#include "stm32f10x_gpio.h"
#include "roller.h"

//滚轮控制初始化
void Roller_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    //初始化时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    //初始化IO口
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOE, GPIO_Pin_6);
}

//关滚轮
void Roller_OFF(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_6);
}

//开滚轮
void Roller_ON(void)
{
    GPIO_SetBits(GPIOE, GPIO_Pin_6);
}

