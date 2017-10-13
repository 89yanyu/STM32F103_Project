#include "stm32f10x_gpio.h"
#include "key.h"

void Key_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

u8 KEY_GetState()
{
    u8 State = 0;
    if (GPIO_ReadInputDataBit(GPIO_WK_UP) == 1) State |= WK_UP;
    if (GPIO_ReadInputDataBit(GPIO_KEY_0) == 0) State |= KEY_0;
    if (GPIO_ReadInputDataBit(GPIO_KEY_1) == 0) State |= KEY_1;
    if (GPIO_ReadInputDataBit(GPIO_KEY_2) == 0) State |= KEY_2;

    return State;
}

