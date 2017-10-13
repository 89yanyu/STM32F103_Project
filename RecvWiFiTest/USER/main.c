#include <string.h>
#include "wifi.h"
#include "delay.h"
#include "led.h"
#include "usart.h"

char *str = "TestLink";

int main()
{
    LED_Init();
    Delay_Init();
    UIO_Init(115200);

    LED_ON(LED_0);

    while (WiFi_Init() != 0)
    {
        Delay_ms_Lazy(100);
        UIO_printf("WiFi init failed.\r\n");
    }
    UIO_printf("WiFi init succeed.\r\n");

    for (;;)
    {
        LED_ON(LED_0);
        while (WiFi_Connect() != 0)
        {
            Delay_ms_Lazy(100);
            LED_Change(LED_0);
        UIO_printf("WiFi connect failed.\r\n");
    }
    UIO_printf("WiFi connect succeed.\r\n");
        LED_OFF(LED_0);
        LED_ON(LED_1);

        while (WiFi_Send(str, strlen(str)) == 0)
        {
            Delay_ms_Lazy(1000);
            while (!WiFi_SendFinish())
            {
                Delay_ms_Lazy(1);
            }
            LED_Change(LED_1);
            UIO_printf("WiFi test succeed.\r\n");
        }
        UIO_printf("WiFi test failed.\r\n");
        LED_OFF(LED_1);
    }
    return 0;
}
