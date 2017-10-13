#include <string.h>
#include "wifi.h"
#include "usart.h"
#include "delay.h"
#include "led.h"

//0: ok 1: Time out
u8 WiFi_Wait(char *stdstr, int timeout)
{
    char buff[32];
    int i = 1;
    int len = strlen(stdstr);
    SysTick_Type *Tick = SysTick;
    u32 ms = 8000, temp;
    ms *= 10;

    for (i = 1; i <= timeout * 100; i++)
    {
        Tick->LOAD = ms;
        Tick->VAL = 0;
        Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

        do {
            if (UIO_hasNext())
            {
                do
                {
                    memset(buff, 0, sizeof(buff));
                    UIO_read(buff, 32);
                    //UIO_printf("%32s\r\n", buff);
                    if (memcmp(buff, stdstr, len) == 0)
                    {
                        Tick->VAL = 0;
                        Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
                        return 0;
                    }
                }
                while (UIO_hasNext());
            }
            temp = Tick->CTRL;
        } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
                 !(temp & SysTick_CTRL_COUNTFLAG_Msk));
    }

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    return 1;
}

u8 WiFi_Init()
{
    UIO_Init(115200);
    UIO_printf("AT+RST\r\n");
    if (WiFi_Wait("ready", 10)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CWMODE_CUR=1\r\n");
    if (WiFi_Wait("OK", 3)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PSWD);
    if (WiFi_Wait("OK", 10)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", WIFI_SRVIP, WIFI_SRVPORT);
    if (WiFi_Wait("OK", 3)) return 1;
    return 0;
}

u8 WiFi_Send(void *buff, int len)
{
    UIO_printf("AT+CIPSEND=%d\r\n", len);
    if (WiFi_Wait("OK", 10)) return 1;
    //Delay_ms(5);
    LED_ON(LED_0);
    UIO_send(buff, len);
    LED_ON(LED_1);
    if (WiFi_Wait("SEND OK", 10)) return 1;
    //Delay_ms(5);
    return 0;
}
