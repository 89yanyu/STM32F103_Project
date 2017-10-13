#include <string.h>
#include <stdio.h>

#include "timer.h"
#include "usart.h"
#include "encoder.h"
#include "pwm.h"
#include "wifi.h"
#include "delay.h"

double Kp = 1., Ki = 0, Kd = 0, sigmaError, diffError;

#define MAX_Ctrl 65535
#define MIN_Ctrl 0
#define TargetSpeed 70.
#define FullSpeed 80.

int nowCtrl, lastError = TargetSpeed;
#define lrint(x) ((int)(x + 0.5))
int spdcnt = 0;
char wifisendbuff[2048];
int wifisenddatalen;
char wifirecvbuff[32];
int wifirecvdatalen;

void GetSendSpeed()
{
    int speed = Encoder_GetSpeed();
    int error = lrint(TargetSpeed - speed);

    spdcnt++;

    sigmaError += error;
    diffError = error - lastError;
    nowCtrl += lrint(error * Kp + sigmaError * Ki + diffError * Kd);
    lastError = error;
    if (nowCtrl > MAX_Ctrl) nowCtrl = MAX_Ctrl;
    if (nowCtrl < MIN_Ctrl) nowCtrl = MIN_Ctrl;

    wifisenddatalen += sprintf(wifisendbuff + wifisenddatalen, "%d %d\r\n", speed, 20);

    PWM_Change(nowCtrl, 0);
}

void StopTest()
{
    UIO_printf("Stop test.\r\n");
    PWM_Stop();
    Encoder_Stop();
    spdcnt = 0;
    TIM_Cmd(TIM5, DISABLE);
    UIO_printf("Send %d data.\r\n", wifisenddatalen);
    UIO_printf("Send result: %d\r\n", WiFi_Send(wifisendbuff, wifisenddatalen));
    wifisenddatalen = 0;
}

int main()
{
    UIO_Init(115200);
    Encoder_Init();
    Timer_Init();
    PWM_Init();
    Delay_Init();

    TIM_Cmd(TIM5, DISABLE);
    Timer_SetHandler(GetSendSpeed);

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
                    if ((wifirecvbuff[2] == 'A') && (spdcnt == 0))
                    {
                        UIO_printf("Start test.\r\n");
                        wifisenddatalen = 0;
                        sigmaError = 0;
                        TIM_Cmd(TIM5, ENABLE);
                        nowCtrl = lrint(65535 * (TargetSpeed / FullSpeed));
                        PWM_Change(nowCtrl, 0);
                        Encoder_Start();
                    }
                    else if ((wifirecvbuff[2] == 'O') && (spdcnt > 0)) //STOP
                    {
                        StopTest();
                    }
                }
                else if ((wifirecvbuff[1] == 'E') && (spdcnt == 0)) //SETPID
                {
                    sscanf(wifirecvbuff, "%*s %lf %lf %lf", &Kp, &Ki, &Kd);
                    wifisenddatalen = sprintf(wifisendbuff, "PID Changed! Kp = %g, Ki = %g, Kd = %g\r\n", Kp, Ki, Kd);
                    UIO_printf("Send result: %d\r\n", WiFi_Send(wifisendbuff, wifisenddatalen));
                }
            }
            if (spdcnt >= 100)
            {
                StopTest();
            }
            if (WiFi_isClosed())
            {
                break;
            }
        }
    }
}
