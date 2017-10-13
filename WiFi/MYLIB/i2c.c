#include "i2c.h"
#include "delay.h"

#define SDA_IN() \
{ \
    GPIOB->CRL &= 0x0FFFFFFF; \
    GPIOB->CRL |= (u32)8 << 28; \
}

#define SDA_OUT() \
{ \
    GPIOB->CRL &= 0x0FFFFFFF; \
    GPIOB->CRL |= (u32)3 << 28; \
}

#define SCL_GPIO GPIOB, GPIO_Pin_6

#define SDA_GPIO GPIOB, GPIO_Pin_7

void I2C_MyInit()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_SetBits(SDA_GPIO);
    GPIO_SetBits(SCL_GPIO);
}

void I2C_Start()
{
    SDA_OUT();
    GPIO_SetBits(SDA_GPIO);
    GPIO_SetBits(SCL_GPIO);
    Delay_us_Lazy(4);
    GPIO_ResetBits(SDA_GPIO);
    Delay_us_Lazy(4);
    GPIO_ResetBits(SCL_GPIO);
}

void I2C_Stop()
{
    SDA_OUT();
    GPIO_ResetBits(SCL_GPIO);
    GPIO_ResetBits(SDA_GPIO);
    Delay_us_Lazy(4);
    GPIO_SetBits(SCL_GPIO);
    GPIO_SetBits(SDA_GPIO);
    Delay_us_Lazy(4);
}

u8 I2C_WaitforACK()
{
    u8 time = 0;
    SDA_IN();
    GPIO_SetBits(SDA_GPIO);
    Delay_us_Lazy(1);
    GPIO_SetBits(SCL_GPIO);
    Delay_us_Lazy(1);
    while (GPIO_ReadInputDataBit(SDA_GPIO))
    {
        time++;
        if (time > 250)
        {
            I2C_Stop();
            return 1;
        }
    }
    GPIO_ResetBits(SCL_GPIO);
    return 0;
}

void I2C_ACK()
{
    GPIO_ResetBits(SCL_GPIO);
    SDA_OUT();
    GPIO_ResetBits(SDA_GPIO);
    Delay_us_Lazy(2);
    GPIO_SetBits(SCL_GPIO);
    Delay_us_Lazy(2);
    GPIO_ResetBits(SCL_GPIO);
}

void I2C_NACK()
{
    GPIO_ResetBits(SCL_GPIO);
    SDA_OUT();
    GPIO_SetBits(SDA_GPIO);
    Delay_us_Lazy(2);
    GPIO_SetBits(SCL_GPIO);
    Delay_us_Lazy(2);
    GPIO_ResetBits(SCL_GPIO);
}

void I2C_WriteByte(u8 txd)
{
    u8 t;
    SDA_OUT();
    GPIO_ResetBits(SCL_GPIO);
    for (t = 8; t > 0; t--)
    {
        if ((txd >> (t - 1)) & 1)
        {
            GPIO_SetBits(SDA_GPIO);
        }
        else
        {
            GPIO_ResetBits(SDA_GPIO);
        }
        Delay_us_Lazy(2);
        GPIO_SetBits(SCL_GPIO);
        Delay_us_Lazy(2);
        GPIO_ResetBits(SCL_GPIO);
        Delay_us_Lazy(2);
    }
}
u8 I2C_ReadByte()
{
    u8 t, recv = 0;
    SDA_IN();
    for (t = 0; t < 8; t++)
    {
        GPIO_ResetBits(SCL_GPIO);
        Delay_us_Lazy(2);
        GPIO_SetBits(SCL_GPIO);
        recv <<= 1;
        if (GPIO_ReadInputDataBit(SDA_GPIO))
        {
            recv++;
        }
        Delay_us_Lazy(1);
    }
    I2C_ACK();
    return recv;
}

