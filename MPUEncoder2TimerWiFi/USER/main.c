#include "mpu6050.h"
#include "usart.h"
#include "delay.h"
#include "wifi.h"
#include "led.h"
#include "timer.h"
#include "encoder.h"

#include "string.h"
#include "stdio.h"

char buff[1024];
char sbuf[1024];

typedef struct
{
    u32 time;
    short gx, gy, gz, ax, ay, az, tp;
    short sp;
} SendDataStruct;

typedef struct
{
    u32 tick;
    SendDataStruct data;
    char corr;
} SendDataPkt;
SendDataPkt senddata;

#define hton16(x) (((x) >> 8) | (((x) & 0x00ff) << 8))

#define hton32(x) (hton16((x) >> 16) | (hton16((x) & 0xffff) << 16))

u16 errid1 = 0, errid2 = 0;
short sendlen = 0, len = 0;

void GetMPUData()
{
    static u32 time = 0;
    int index = 0;
    /*
    errid1 = MPU6050_GetTemperature(&senddata.tp) << 2 ||
             MPU6050_GetGyroscope(&senddata.gx, &senddata.gy, &senddata.gz) << 1 ||
             MPU6050_GetAccelerometer(&senddata.ax, &senddata.ay, &senddata.az);
    */
    memset((char*)&senddata, 0, sizeof(senddata));
    errid1 = MPU6050_GetAllData(&senddata.data.ax, &senddata.data.ay, &senddata.data.az,
                                &senddata.data.tp,
                                &senddata.data.gx, &senddata.data.gy, &senddata.data.gz);
    senddata.data.sp = Encoder_GetSpeed();
    time++;
    senddata.data.time = time;
    if (errid1)
    {
        Timer_SetHandler(NULL);
        return;
    }

    if ((sendlen > 0) && (len > 900))
    {
        return;
    }

    senddata.tick = 0x01770277;
    senddata.corr = 0;

    senddata.data.time = hton32(senddata.data.time);
    senddata.data.gx = hton16(senddata.data.gx);
    senddata.data.gy = hton16(senddata.data.gy);
    senddata.data.gz = hton16(senddata.data.gz);
    senddata.data.ax = hton16(senddata.data.ax);
    senddata.data.ay = hton16(senddata.data.ay);
    senddata.data.az = hton16(senddata.data.az);
    senddata.data.tp = hton16(senddata.data.tp);
    senddata.data.sp = hton16(senddata.data.sp);

    for (index = 0; index < sizeof(SendDataStruct); index++)
    {
        senddata.corr ^= *(((char*)(void*)&(senddata.data)) + index);
    }

    senddata.tick = hton32(senddata.tick);

    memcpy(buff + len, (char*)&senddata, sizeof(senddata));
    len += sizeof(senddata);
    //len += sprintf(buff + len,
    //               "%d %d %d %d %d %d %d %d %u\r\n",
    //               senddata.gx, senddata.gy, senddata.gz,
    //               senddata.ax, senddata.ay, senddata.az,
    //               senddata.tp, senddata.sp, senddata.time);

    if ((len > 500) && (sendlen == 0))
    {
        sendlen = len;
        memcpy(sbuf, buff, sendlen);
        len = 0;
    }

    LED_Change(LED_1);

}

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    Delay_Init();
    LED_Init();
    Timer_Init();
    Encoder_Init();
    Encoder_Stop();

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
        Encoder_GetSpeed();
        for (;;)
        {
            if (sendlen > 0)
            {
                errid2 = WiFi_Send(sbuf, sendlen);
                sendlen = 0;
            }
            if (errid1 || errid2)
            {
                LED_OFF(LED_1);
                Encoder_Stop();
                break;
            }
        }
        if (errid1)
        {
            break;
        }
    }
}

