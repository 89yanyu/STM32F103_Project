#include "stm32f10x_gpio.h"
#include "led.h"

void LED_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_5);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
}

void LED_OFF(u8 led)
{
    if (LED_0 & led)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
    }
    if (LED_1 & led)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_5);
    }
}

void LED_ON(u8 led)
{
    if (LED_0 & led)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    }
    if (LED_1 & led)
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_5);
    }
}

void LED_Change(u8 led)
{
    if (LED_0 & led)
    {
        if (1 == GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5))
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        }
        else
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_5);
        }
    }
    if (LED_1 & led)
    {
        if (1 == GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_5))
        {
            GPIO_ResetBits(GPIOE, GPIO_Pin_5);
        }
        else
        {
            GPIO_SetBits(GPIOE, GPIO_Pin_5);
        }
    }
}

