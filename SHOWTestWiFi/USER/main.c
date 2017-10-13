#include <string.h>
#include <stdio.h>

#include "usart.h"
#include "wifi.h"
#include "delay.h"
#include "led.h"
#include "pwm.h"

#define lrint(x) ((int)(x + 0.5))

char wifirecvbuff[32];
int wifirecvdatalen;

extern char WIFI_SSID[32];

extern char WIFI_PSWD[32];

extern char WIFI_SRVIP[32];

u8 ok = 0;
#define readfromUIO(str) ok = 0; \
    do \
    { \
        UIO_printf(#str"?\r\n");\
        Delay_ms_Lazy(3000);\
        if (UIO_hasNext())\
        {\
            ok = 1; \
            UIO_gets(str);\
        }\
    }\
    while (ok == 0)

int main()
{
    UIO_Init(115200);
    PWM_Init();
    PWM_Change(0, 0);
    Delay_Init();
    LED_Init();

    UIO_printf("Init finish.\r\n");
    
    readfromUIO(WIFI_SSID);
    UIO_printf("#%s#\r\n", WIFI_SSID);
    readfromUIO(WIFI_PSWD);
    UIO_printf("@%s@\r\n", WIFI_PSWD);
    readfromUIO(WIFI_SRVIP);
    UIO_printf("&%s&\r\n", WIFI_SRVIP);

    while (WiFi_Init())
    {
        Delay_ms_Lazy(10);
    }
    UIO_SetAsync(ENABLE);

    for(;;)
    {
        while (WiFi_Connect() != 0)
        {
            UIO_printf("Connect failed.\r\n");
            Delay_ms_Dosth(100, UIO_AsyncSend);
        }
        UIO_printf("Connect succeed.\r\n");
        for(;;)
        {
            UIO_AsyncSend();
            wifirecvdatalen = WiFi_Read(wifirecvbuff);
            //UIO_printf("%d\r\n", wifirecvdatalen);
            if (wifirecvdatalen > 0)
            {
                UIO_printf("Recv (%d):", wifirecvdatalen);
                UIO_send(wifirecvbuff, wifirecvdatalen);
                if (wifirecvbuff[1] == 'T') //START
                {
                    if (wifirecvbuff[2] == 'A')
                    {
                        PWM_Change(6000, 6000);
                        UIO_send("Start successed\r\n", 17);
                    }
                    else if (wifirecvbuff[2] == 'O') //STOP
                    {
                        PWM_Change(0, 0);
                        UIO_send("Stop successed\r\n", 16);
                    }
                }
            }
            if (WiFi_isClosed())
            {
                break;
            }
        }
    }
}
