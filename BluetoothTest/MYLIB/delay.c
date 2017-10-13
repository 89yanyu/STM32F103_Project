/**************************************
* ���ļ�Ϊ��ʱ��غ�����
* ���������ԭ��
**************************************/
#include "delay.h"

//���붨ʱ��΢�붨ʱ�ı���
#define delayms_fac 8000
#define delayus_fac 8

//��ʼ��SysTick���ƺ��ô�����
void Delay_Init()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

//��ʱ���Ժ���Ϊ��λ
void Delay_ms(u32 ms)
{
    //��ʱ����Systick����ʱ
    u32 temp;
    SysTick_Type *Tick = SysTick;
    //���ݱ��ʼ���ʵ�ʼ���ֵ
    ms *= delayms_fac;
    //���ؼ���ֵ
    Tick->LOAD = ms;
    Tick->VAL = 0;
    //����SysTick
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        //���ϻ�ȡ��ֱ��SysTick���رգ����ߴﵽԤ�����ֵ
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    //��ԭ����ֵ���ر�SysTick
    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//��ʱ����΢��Ϊ��λ
//����Delay_ms���ڴ˲���׸��
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

//������ʱ���Ժ���Ϊ��λ
void Delay_ms_Lazy(u32 ms)
{
    u32 temp;
    //��ѭ����ʱ
    //��ѭ�����һ����1ms
    //һ��ִ��ms��
    do {
        temp = delayms_fac;
        do {
            temp--;
        } while (temp != 0);
        ms--;
    } while (ms != 0);
}

//������ʱ����΢��Ϊ��λ
//����Delay_ms_Lazy���ڴ˲���׸��
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

//��������ʱ������ʱ��ͬʱִ��sth
//�������豣֤��sth���Կ���ִ��
//������ܳ�����ʱ��׼ȷ
void Delay_ms_Dosth(u32 ms, void(*sth)())
{
    u32 temp;
    SysTick_Type *Tick = SysTick;
    ms *= delayms_fac;
    Tick->LOAD = ms;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        //�ڵȴ�ʱ��ִ��sth
        sth();
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
             !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

