#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "usart.h"
#include "delay.h"
#include "i2c.h"
#include "mpu6050.h"
#include "led.h"

int main()
{
    float pitch, roll, yaw;
    int res = 0;
    Delay_Init();
    UIO_Init(115200);
    LED_Init();
    
    LED_ON(LED_0);
    while(res = mpu_dmp_init())
    {
        Delay_ms(200);
        LED_Change(LED_0);
        UIO_printf("res = %d\r\n", res);
    }
    UIO_SetAsync(ENABLE);
    LED_OFF(LED_0);
    LED_ON(LED_1);

    while(1)
    {
        if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
        {
            UIO_printf("%.8f %.8f %.8f\r\n", pitch, roll, yaw);
        }
        else
        {
            UIO_printf("Error\r\n");
        }
        Delay_ms_Dosth(10, UIO_AsyncSend);
    }
}
