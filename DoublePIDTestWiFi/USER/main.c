#include <string.h>
#include <stdio.h>

#include "timer.h"
#include "usart.h"
#include "encoder.h"
#include "encoder2.h"
#include "pwm.h"
#include "wifi.h"
#include "delay.h"

#define lrint(x) ((int)(x + 0.5))

char wifirecvbuff[32];
int wifirecvdatalen;

int main()
{
    UIO_Init(115200);
    PWM_Init();
    Delay_Init();

    UIO_printf("Init finish.\r\n");
    UIO_SetAsync(ENABLE);

    while (WiFi_Init()) Delay_ms_Dosth(10, UIO_AsyncSend);

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
                        UIO_send("Start successed\r\n", 17);
                    }
                    else if (wifirecvbuff[2] == 'O') //STOP
                    {
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
