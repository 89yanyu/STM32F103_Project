/**************************************
* 本文件为延时相关函数，
* 借鉴自正点原子
**************************************/
#include "delay.h"

//毫秒定时和微秒定时的倍率
#define delayms_fac 8000
#define delayus_fac 8

//初始化SysTick，似乎用处不大？
void Delay_Init()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

//延时，以毫秒为单位
void Delay_ms(u32 ms)
{
    //计时采用Systick倒计时
    u32 temp;
    SysTick_Type *Tick = SysTick;
    //根据倍率计算实际计数值
    ms *= delayms_fac;
    //加载计数值
    Tick->LOAD = ms;
    Tick->VAL = 0;
    //开启SysTick
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        //不断获取，直到SysTick被关闭，或者达到预设计数值
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    //还原计数值，关闭SysTick
    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//延时，以微秒为单位
//过程Delay_ms，在此不再赘述
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

//简易延时，以毫秒为单位
void Delay_ms_Lazy(u32 ms)
{
    u32 temp;
    //以循环计时
    //外循环完成一次是1ms
    //一共执行ms次
    do {
        temp = delayms_fac;
        do {
            temp--;
        } while (temp != 0);
        ms--;
    } while (ms != 0);
}

//简易延时，以微秒为单位
//过程Delay_ms_Lazy，在此不再赘述
void Delay_us_Lazy(u32 us)
{
    u32 temp;
    do {
        temp = delayus_fac;
        do {
            temp--;
        } while (temp != 0);
        us--;
    } while (us != 0);
}

//升级版延时，在延时的同时执行sth
//调用者需保证，sth可以快速执行
//否则可能出现延时不准确
void Delay_ms_Dosth(u32 ms, void(*sth)())
{
    u32 temp;
    SysTick_Type *Tick = SysTick;
    ms *= delayms_fac;
    Tick->LOAD = ms;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        //在等待时，执行sth
        sth();
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

