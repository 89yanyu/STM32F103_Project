#include <string.h>
#include <stdlib.h>
#include "wifi.h"
#include "usart.h"
#include "delay.h"
#include "led.h"

u8 isOnline[5] = {0, 0, 0, 0, 0};

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
    static u8 first = 1;
    if (first)
    {
        UIO_Init(115200);
        first = 0;
    }
    memset(isOnline, 0, sizeof(isOnline));
    UIO_printf("AT+RST\r\n");
    if (WiFi_Wait("ready", 10)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CWMODE_CUR=2\r\n");
    if (WiFi_Wait("OK", 3)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CIPMUX=1\r\n");
    if (WiFi_Wait("OK", 3)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CWSAP=\"%s\",\"%s\",6,4\r\n", WIFI_SSID, WIFI_PSWD);
    if (WiFi_Wait("OK", 3)) return 1;
    Delay_ms(300);
    UIO_printf("AT+CIPSERVER=1,%d\r\n", WIFI_SRVPORT);
    if (WiFi_Wait("OK", 10)) return 1;
    return 0;
}

u8 WiFi_isAnyoneConnected()
{
    return (isOnline[0] ||
            isOnline[1] ||
            isOnline[2] ||
            isOnline[3] ||
            isOnline[4]);

}

u8 WiFi_ReduceConnect()
{
    char buff[32];
    int p, id, ret = 0;
    while (UIO_hasNext())
    {
        UIO_scanf("%s", buff);
        if (buff[0] == '+') continue;
        p = strstr(buff, ",") - buff;
        id = atoi(buff);
        if (strcmp(buff + (p + 1), "CONNECT") == 0)
        {
            if (isOnline[id] == 0)
            {
                ret++;
            }
            isOnline[id] = 1;
        }

        if (strcmp(buff + (p + 1), "CLOSED") == 0)
        {
            if (isOnline[id] == 1)
            {
                ret--;
            }
            isOnline[id] = 0;
        }

    }
    return ret;
}

u8 WiFi_Send(char *buff, int len)
{
    int id = 0; /*
    for (id = 0; id < 5; id++)
    {
        if (!isOnline[id])
        {
            continue;
        } */
        UIO_printf("AT+CIPSEND=%d,%d\r\n", id, len);
        if (WiFi_Wait("OK", 5)) return 1;
        //Delay_ms(5);
        UIO_send(buff, len);
        if (WiFi_Wait("SEND OK", 10)) return 1;
        //Delay_ms(5);
    //}
    return 0;
}
