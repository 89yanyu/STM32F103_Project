#include "mpu6050.h"
#include "i2c.h"
#include "delay.h"
#include "usart.h"

#define MPU_ADDR 0X68

u8 MPU6050_WriteOneByte(u8 Addr, u8 Data)
{
    I2C_Start();
    I2C_WriteByte((MPU_ADDR << 1) | 0);
    if (I2C_WaitforACK())
    {
        return 1;
    }
    I2C_WriteByte(Addr);
    I2C_WaitforACK();
    I2C_WriteByte(Data);
    if (I2C_WaitforACK())
    {
        return 1;
    }
    I2C_Stop();

    return 0;
}

u8 MPU6050_ReadOneByte(u8 Addr)
{
    u8 ret = 0;
    I2C_Start();
    I2C_WriteByte((MPU_ADDR << 1) | 0);
    I2C_WaitforACK();
    I2C_WriteByte(Addr);
    I2C_WaitforACK();
    I2C_Start();
    I2C_WriteByte((MPU_ADDR << 1) | 1);
    I2C_WaitforACK();
    ret = I2C_ReadByte(0);
    I2C_Stop();

    return ret;
}

u8 MPU6050_ReadBuff(u8 Addr, u8 Len, u8 *Buff)
{
    I2C_Start();
    I2C_WriteByte((MPU_ADDR << 1) | 0);
    if (I2C_WaitforACK())
    {
        return 1;
    }
    I2C_WriteByte(Addr);
    I2C_WaitforACK();
    I2C_Start();
    I2C_WriteByte((MPU_ADDR << 1) | 1);
    I2C_WaitforACK();
    while (Len--)
    {
        *Buff = I2C_ReadByte(Len == 0 ? 0 : 1);
        Buff++;
    }
    I2C_Stop();

    return 0;

}

u8 MPU6050_WriteBuff(u8 Addr, u8 Len, u8 *Buff)
{
    u8 i;
    I2C_Start();
    I2C_WriteByte((MPU_ADDR << 1) | 0);
    if (I2C_WaitforACK())
    {
        return 1;
    }
    I2C_WriteByte(Addr);
    I2C_WaitforACK();
    for (i = 0; i < Len; i++)
    {
        I2C_WriteByte(Buff[i]);
        if (I2C_WaitforACK())
        {
            I2C_Stop();
            return 1;
        }
    }
    I2C_Stop();

    return 0;

}

u8 MPU6050_SetLPF(u16 lpf)
{
    u8 data=0;
    if (lpf>=188)
    {
        data=1;
    }
    else if (lpf>=98)
    {
        data=2;
    }
    else if (lpf>=42)
    {
        data=3;
    }
    else if (lpf>=20)
    {
        data=4;
    }
    else if (lpf>=10)
    {
        data=5;
    }
    else
    {
        data=6;
    }
    return MPU6050_WriteOneByte(MPU6050_CFGREG, data);//ÉèÖÃÊý×ÖµÍÍ¨ÂË²¨Æ÷

}

u8 MPU6050_SetRate(u16 rate)
{
    u8 data;
    if (rate > 1000) rate = 1000;
    if (rate < 4) rate = 4;
    data = 1000 / rate - 1;
    return MPU6050_WriteOneByte(MPU6050_SAMPLERATEREG, data) &&
           MPU6050_SetLPF(rate / 2);
}

u8 MPU6050_Init()
{
    u8 res;
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);

    I2C_MyInit();

    MPU6050_WriteOneByte(MPU6050_PWRMGMT1REG, 0x80);
    Delay_ms_Lazy(100);
    MPU6050_WriteOneByte(MPU6050_PWRMGMT1REG, 0x00);
    Delay_ms_Lazy(100);

    //fsr:0,¡À250dps;1,¡À500dps;2,¡À1000dps;3,¡À2000dps
    MPU6050_WriteOneByte(MPU6050_GYROCFGREG, 3 << 3);

    //fsr:0,¡À2g;1,¡À4g;2,¡À8g;3,¡À16g
    MPU6050_WriteOneByte(MPU6050_ACCELCFGREG, 0 << 3);

    MPU6050_SetRate(50);

    MPU6050_WriteOneByte(MPU6050_INTENREG, 0x00);

    MPU6050_WriteOneByte(MPU6050_USERCTRLREG, 0x00);

    MPU6050_WriteOneByte(MPU6050_FIFOENREG, 0x00);

    MPU6050_WriteOneByte(MPU6050_INTBPCFGREG, 0x80);

    res = MPU6050_ReadOneByte(MPU6050_DEVICEIDREG);
    if (MPU_ADDR == res)
    {
        MPU6050_WriteOneByte(MPU6050_PWRMGMT1REG, 0x01);
        MPU6050_WriteOneByte(MPU6050_PWRMGMT2REG, 0x00);
        MPU6050_SetRate(50);
    }
    else
    {
        return 1;
    }

    return 0;
}

u8 MPU6050_GetTemperature(short *tp)
{
    u8 buff[2];
    u16 raw;

    if (0 == MPU6050_ReadBuff(MPU6050_TEMPOUTHREG, 2, buff))
    {
        raw = ((u16)buff[0] << 8) | buff[1];
        *tp = (3653 + raw / 3.4);
        return 0;
    }

    return 1;
}

u8 MPU6050_GetGyroscope(short *gx, short *gy, short *gz)
{
    u8 buff[6];

    if (0 == MPU6050_ReadBuff(MPU6050_GYROXOUTHREG, 6, buff))
    {
        *gx=((u16)buff[0] << 8) | buff[1];
        *gy=((u16)buff[2] << 8) | buff[3];
        *gz=((u16)buff[4] << 8) | buff[5];
        return 0;
    }
    return 1;
}
u8 MPU6050_GetAccelerometer(short *ax,short *ay,short *az)
{
    u8 buff[6];

    if (0 == MPU6050_ReadBuff(MPU6050_ACCELXOUTHREG, 6, buff))
    {
        *ax=((u16)buff[0] << 8) | buff[1];
        *ay=((u16)buff[2] << 8) | buff[3];
        *az=((u16)buff[4] << 8) | buff[5];
        return 0;
    }
    return 1;
}
u8 MPU6050_GetAllData(short *ax, short *ay, short *az, short *tp, short *gx, short *gy, short *gz)
{
    u8 buff[14];

    if (0 == MPU6050_ReadBuff(MPU6050_ACCELXOUTHREG, 14, buff))
    {
        *ax=((u16)buff[ 0] << 8) | buff[ 1];
        *ay=((u16)buff[ 2] << 8) | buff[ 3];
        *az=((u16)buff[ 4] << 8) | buff[ 5];
        *tp=((u16)buff[ 6] << 8) | buff[ 7];
        *gx=((u16)buff[ 8] << 8) | buff[ 9];
        *gy=((u16)buff[10] << 8) | buff[11];
        *gz=((u16)buff[12] << 8) | buff[13];
        return 0;
    }
    return 1;

}

