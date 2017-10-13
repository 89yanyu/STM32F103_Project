#include "i2c.h"
#include "delay.h"
#include "usart.h"
#include "string.h"

void WriteOneByte(u8 Addr, u8 Data)
{
    I2C_Start();
    I2C_WriteByte(0xA0);
    I2C_WaitforACK();
    I2C_WriteByte(Addr);
    I2C_WaitforACK();
    I2C_WriteByte(Data);
    I2C_WaitforACK();
    I2C_Stop();
    Delay_ms(10);
}

u8 ReadOneByte(u8 Addr)
{
    u8 ret = 0;
    I2C_Start();
    I2C_WriteByte(0xA0);
    I2C_WaitforACK();
    I2C_WriteByte(Addr);
    I2C_WaitforACK();
    I2C_Start();
    I2C_WriteByte(0xA1);
    I2C_WaitforACK();
    ret = I2C_ReadByte();
    I2C_Stop();

    return ret;
}

u8 Check(void)
{
    u8 temp;
    temp=ReadOneByte(255);//避免每次开机都写AT24CXX
    if(temp==0x55)return 0;
    else//排除第一次初始化的情况
    {
        WriteOneByte(255,0x55);
        temp=ReadOneByte(255);
        if(temp==0x55)return 0;
    }
    return 1;
}
int main()
{
    char str[8];
    u8 x;
    u8 top = 0;
    UIO_Init(115200);
    I2C_MyInit();
    Delay_Init();
    UIO_printf("GOGOGO\r\n");
    while (Check())
    {
        UIO_printf("Check fail\r\n");
        Delay_ms(10);
    }
    for(;;)
    {
        UIO_scanf("%s %d", str, &x);
        if (strcmp(str, "PUSH") == 0)
        {
            WriteOneByte(top, x);
            top++;
            UIO_printf("$\r\n");
        }
        else if ((strcmp(str, "POP") == 0) && (top > 0))
        {
            top--;
            x = ReadOneByte(top);
            UIO_printf("%d\r\n", x);
        }
        Delay_ms_Lazy(1);
    }
}
