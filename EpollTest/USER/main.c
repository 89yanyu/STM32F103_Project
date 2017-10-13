#include "mpu6050.h"
#include "usart.h"
#include "delay.h"
#include "wifi.h"
#include "led.h"
#include "timer.h"
#include "epoll.h"

#include "string.h"
#include "stdio.h"

char buff[1024];
char sbuf[1024];

typedef struct
{
    u32 time;
    short gx, gy, gz, ax, ay, az, tp;
} SendDataStruct;
SendDataStruct senddata;

u16 errid1 = 0, errid2 = 0;
short sendlen = 0, len = 0;

void GetMPUData()
{
    static u32 time = 0;
    errid1 = MPU6050_GetAllData(&senddata.ax, &senddata.ay, &senddata.az,
                                &senddata.tp,
                                &senddata.gx, &senddata.gy, &senddata.gz);
    time++;
    senddata.time = time;
    if (errid1)
    {
        Timer_SetHandler(NULL);
        return;
    }

    if ((sendlen > 0) && (len > 900))
    {
        return;
    }

    //memcpy(buff + len, (char*)&senddata, sizeof(senddata));
    //len += sizeof(senddata);
    len += sprintf(buff + len,
                   "%d %d %d %d %d %d %d %u\r\n",
                   senddata.gx, senddata.gy, senddata.gz,
                   senddata.ax, senddata.ay, senddata.az,
                   senddata.tp, senddata.time);

    if ((len > 100) && (sendlen == 0))
    {
        sendlen = len;
        memcpy(sbuf, buff, sendlen);
        len = 0;
    }

    LED_Change(LED_1);

}

int timer_epollfd = -1;
int wifi_epollfd = -1;
void TimerEpollHandler()
{
    if (timer_epollfd != -1)
    {
        Epoll_in(timer_epollfd, NULL, 0);
    }
}
int TimerEpollCallback(void *ptr, int len)
{
    errid1 = MPU6050_GetAllData(&senddata.ax, &senddata.ay, &senddata.az,
                                &senddata.tp,
                                &senddata.gx, &senddata.gy, &senddata.gz);
    if (wifi_epollfd != -1)
    {
        Epoll_in(wifi_epollfd, NULL, 0);
    }
}

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    Delay_Init();
    LED_Init();
    Timer_Init();

    while (MPU6050_Init())
    {
        Delay_ms(100);
    }

    LED_ON(LED_0);
    while (WiFi_Init())
    {
        Delay_ms(100);
    }
    LED_OFF(LED_0);

    for (;;)
    {
        int connecttimes = 1;
        LED_OFF(LED_1);
        LED_ON(LED_0);
        while (WiFi_Connect())
        {
            Delay_ms(10);
            LED_Change(LED_0);
            connecttimes++;
            if (connecttimes >= 10)
            {
                break;
            }
        }

        if (connecttimes >= 10)
        {
            LED_ON(LED_0);
            while (WiFi_Init())
            {
                Delay_ms(100);
            }
            LED_OFF(LED_0);
            continue;
        }

        LED_OFF(LED_0);
        LED_ON(LED_1);
        Timer_SetHandler(GetMPUData);
        for (;;)
        {
            if (sendlen > 0)
            {
                errid2 = WiFi_Send(sbuf, sendlen);
                sendlen = 0;
            }
            if (errid1 || errid2)
            {
                break;
            }
        }
        if (errid1)
        {
            break;
        }
    }
}

