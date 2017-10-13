#include "usart.h"
#include "delay.h"
#include "led.h"
#include "wifi.h"

int main()
{
    Delay_Init();
    LED_Init();
    for(;;)
    {
        LED_ON(LED_0);
        while (WiFi_Init())
        {
            Delay_ms(100);
        }
        LED_OFF(LED_0);

        for(;;)
        {
            LED_ON(LED_1);
            if (WiFi_Send("123456\r\n", 8))
            {
                LED_OFF(LED_1);
                break;
            }
            LED_OFF(LED_1);
        }
    }
}
