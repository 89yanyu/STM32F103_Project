#include "mpu6050.h"
#include "usart.h"
#include "delay.h"
#include "wifi.h"
#include "led.h"

#include "stdio.h"

char buff[128];

int main()
{
    short tp, gx, gy, gz, ax, ay, az, errid = 0, len;
    int times = 0;

    Delay_Init();
    LED_Init();

    while (MPU6050_Init())
    {
        Delay_ms(100);
    }

    for (;;)
    {
        LED_ON(LED_0);
        while (WiFi_Init())
        {
            Delay_ms(100);
        }

        LED_ON(LED_1);
        while (!WiFi_isAnyoneConnected())
        {
            LED_Change(LED_0);
            if (WiFi_ReduceConnect() > 0)
            {
                break;
            }
            Delay_ms(100);
        }
        LED_OFF(LED_0);
        for (;;)
        {
            errid = MPU6050_GetTemperature(&tp) << 2 ||
                    MPU6050_GetGyroscope(&gx, &gy, &gz) << 1 ||
                    MPU6050_GetAccelerometer(&ax, &ay, &az);
            if (errid)
            {
                break;
            }
            LED_ON(LED_1);
            len = sprintf(buff, "%d %d %d %d %d %d %d %d\r\n", gx, gy, gz, ax, ay, az, tp, errid);
            errid = WiFi_Send(buff, len);
            LED_OFF(LED_1);
            if (errid)
            {
                break;
            }
        }
        errid = 0;
    }
}

