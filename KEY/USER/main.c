#include "stm32f10x_gpio.h"

void Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
}

void Sleep(u32 ms)
{
    u32 temp;
    SysTick_Type *Tick = SysTick;
    ms *= 8000;
    Tick->LOAD = ms;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

/* 4-0: KEY0 KEY1 KEY2 WKUP */
#define KEY0 0x8
#define KEY1 0x4
#define KEY2 0x2
#define WKUP 0x1

u8 KEY_State()
{
    u8 State = 0;
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1) State |= WKUP;
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0) State |= KEY2;
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0) State |= KEY1;
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0) State |= KEY0;

    return State;
}

void ChangeLED0()
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

void ChangeLED1()
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

int main()
{
    u8 LastTempState = 0, NowTempState = 0, ChangeState = 0, KeyUp = 0xF;
    Init();

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
    Sleep(500);
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
    Sleep(500);
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
    Sleep(500);
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
    Sleep(500);
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
    Sleep(500);
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
    while (1)
    {
        NowTempState = KEY_State();
        ChangeState = KeyUp & LastTempState & NowTempState;
        KeyUp = (~LastTempState & ~NowTempState) | 
			          ((~LastTempState | ~NowTempState) & KeyUp);
				LastTempState = NowTempState;

        if (ChangeState & KEY2) ChangeLED0();
        if (ChangeState & KEY0) ChangeLED1();
        if (ChangeState & KEY1)
        {
            ChangeLED0();
            ChangeLED1();
        }
        if (ChangeState & WKUP)
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            GPIO_ResetBits(GPIOE, GPIO_Pin_5);
        }
        Sleep(10);
    }
}
