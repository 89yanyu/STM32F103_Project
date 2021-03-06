#include "delay.h"

#define delayms_fac 8000
#define delayus_fac 8

void Delay_Init()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

void Delay_ms(u32 ms)
{
    u32 temp;
    SysTick_Type *Tick = SysTick;
    ms *= delayms_fac;
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

void Delay_us(u32 us)
{
    u32 temp;
    SysTick_Type *Tick = SysTick;
    us *= delayus_fac;
    Tick->LOAD = us;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void Delay_ms_Lazy(u32 ms)
{
    u32 temp;
    do {
        temp = delayms_fac;
        do {
            temp--;
        } while (temp != 0);
    } while (ms != 0);
}

void Delay_us_Lazy(u32 us)
{
    u32 temp;
    do {
        temp = delayus_fac;
        do {
            temp--;
        } while (temp != 0);
    } while (us != 0);
}

void Delay_ms_Dosth(u32 ms, void(*sth)())
{
    u32 temp;
    SysTick_Type *Tick = SysTick;
    ms *= delayms_fac;
    Tick->LOAD = ms;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        sth();
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

