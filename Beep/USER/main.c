#include "stm32f10x_gpio.h"

void BEEP_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

void Delay_ms(u32 ms)
{
    u32 temp;
    SysTick_Type *Tick = SysTick;

    ms *= 8000;
    Tick->LOAD = ms;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        temp = Tick->CTRL;
    }
    while ((temp & SysTick_CTRL_ENABLE_Msk) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->CTRL &= !SysTick_CTRL_ENABLE_Msk;
    Tick->VAL = 0;
}

int main()
{
    BEEP_Init();

    while (1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        Delay_ms(500);
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        Delay_ms(500);
    }

}
