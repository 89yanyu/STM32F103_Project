#include "mpu6050.h"
#include "usart.h"
#include "delay.h"

void usart1_report_imu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz)
{
    u8 tbuf[28];
    u8 i;
    u8 crc = 0;
    for(i=0;i<28;i++)tbuf[i]=0;//?0
    tbuf[0]=(aacx>>8)&0XFF;
    tbuf[1]=aacx&0XFF;
    tbuf[2]=(aacy>>8)&0XFF;
    tbuf[3]=aacy&0XFF;
    tbuf[4]=(aacz>>8)&0XFF;
    tbuf[5]=aacz&0XFF;
    tbuf[6]=(gyrox>>8)&0XFF;
    tbuf[7]=gyrox&0XFF;
    tbuf[8]=(gyroy>>8)&0XFF;
    tbuf[9]=gyroy&0XFF;
    tbuf[10]=(gyroz>>8)&0XFF;
    tbuf[11]=gyroz&0XFF;
    crc += 0x88;
    UIO_printf("%c", 0x88);
    crc += 0xAF;
    UIO_printf("%c", 0xAF);
    crc += 28;
    UIO_printf("%c", 28);
    for (i = 0; i < 28; i++)
    {
        UIO_printf("%c", tbuf[i]);
        crc += tbuf[i];
    }
    UIO_printf("%c", crc);
}
int main()
{
    Delay_Init();
    UIO_Init(115200);

    while (MPU6050_Init())
    {
        Delay_ms(200);
    }

    UIO_SetAsync(ENABLE);
    for (;;)
    {
        short tp, gx, gy, gz, ax, ay, az;
        Delay_ms_Dosth(20, UIO_AsyncSend);
        if (MPU6050_GetTemperature(&tp) ||
            MPU6050_GetGyroscope(&gx, &gy, &gz) ||
            MPU6050_GetAccelerometer(&ax, &ay, &az))
        {
            //UIO_printf("Something wrong...\r\n");
        }
        else
        {
            //usart1_report_imu(ax, ay, az, gx, gy, gz);
            UIO_printf("%d %d %d %d %d %d %d\r\n", tp, gx, gy, gz, ax, ay, az);
        }
    }
}
