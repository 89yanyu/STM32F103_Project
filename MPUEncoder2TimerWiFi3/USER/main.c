#include "mpu6050.h"
#include "usart.h"
#include "delay.h"
#include "wifi.h"
#include "led.h"
#include "timer.h"
#include "encoder.h"

#include "string.h"
#include "stdio.h"

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
SendDataPkt senddata[32];

#define hton16(x) (((x) >> 8) | (((x) & 0x00ff) << 8))

#define hton32(x) (hton16((x) >> 16) | (hton16((x) & 0xffff) << 16))

u16 errid1 = 0, errid2 = 0;
u8 sendcnt = 0;

void GetMPUData()
{
    static u32 time = 0;
    int index = 0;

    memset((char*)&(senddata[sendcnt]), 0, sizeof(SendDataPkt));
    errid1 = MPU6050_GetAllData(&senddata[sendcnt].data.ax, &senddata[sendcnt].data.ay, &senddata[sendcnt].data.az,
                                &senddata[sendcnt].data.tp,
                                &senddata[sendcnt].data.gx, &senddata[sendcnt].data.gy, &senddata[sendcnt].data.gz);
    senddata[sendcnt].data.sp = Encoder_GetSpeed();
    time++;
    senddata[sendcnt].data.time = time;
    if (errid1)
    {
        Timer_SetHandler(NULL);
        return;
    }

    senddata[sendcnt].tick = 0x01770277;
    senddata[sendcnt].corr = 0;

    senddata[sendcnt].data.time = hton32(senddata[sendcnt].data.time);
    senddata[sendcnt].data.gx = hton16(senddata[sendcnt].data.gx);
    senddata[sendcnt].data.gy = hton16(senddata[sendcnt].data.gy);
    senddata[sendcnt].data.gz = hton16(senddata[sendcnt].data.gz);
    senddata[sendcnt].data.ax = hton16(senddata[sendcnt].data.ax);
    senddata[sendcnt].data.ay = hton16(senddata[sendcnt].data.ay);
    senddata[sendcnt].data.az = hton16(senddata[sendcnt].data.az);
    senddata[sendcnt].data.tp = hton16(senddata[sendcnt].data.tp);
    senddata[sendcnt].data.sp = hton16(senddata[sendcnt].data.sp);

    for (index = 0; index < sizeof(SendDataStruct); index++)
    {
        senddata[sendcnt].corr ^= *(((char*)(void*)&(senddata[sendcnt].data)) + index);
    }

    senddata[sendcnt].tick = hton32(senddata[sendcnt].tick);

    sendcnt++;
    if ((WiFi_SendFinish()) && (sendcnt > 19))
    {
        errid2 = WiFi_Send((char*)senddata, sizeof(SendDataPkt) * sendcnt);
        sendcnt = 0;
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
    UIO_Init(115200);
    UIO_printf("START\r\n");

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
            if (errid1 || errid2)
            {
                UIO_printf("MiaoMiaoMiao\r\n");
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

