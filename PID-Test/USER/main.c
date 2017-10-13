#include "timer.h"
#include "usart.h"
#include "encoder.h"
#include "pwm.h"
#include <string.h>

#define SAVECOUNT 10
int sumsqr = 0, sum = 0, norm = 0;
int svspd[SAVECOUNT], tail;
#define sqr(x) ((x)*(x))
char buff[8];
double Ki, Kp = 1., Kd, sigmaError, diffError;
u8 state = 0;

#define MAX_Ctrl 65535
#define MIN_Ctrl 0
#define TargetSpeed 70.
#define FullSpeed 80.

#define lrint(x) ((int)((x) + 0.5))

int nowCtrl, lastError = 100000;

void GetSendSpeed()
{
    int speed = Encoder_GetSpeed();
    int error = lrint(TargetSpeed - speed);

    sum -= svspd[tail % SAVECOUNT];
    sumsqr -= sqr(svspd[tail % SAVECOUNT]);

    svspd[tail % SAVECOUNT] = speed;
    sum += svspd[tail % SAVECOUNT];
    sumsqr += sqr(svspd[tail % SAVECOUNT]);
    tail++;

    norm = sumsqr * SAVECOUNT - sqr(sum);

    sigmaError += error;
    if (lastError < 100000)
    {
        diffError = error - lastError;
    }
    else
    {
        diffError = 0;
    }
    nowCtrl += lrint(error * Kp + sigmaError * Ki + diffError * Kd);
    lastError = error;
    if (nowCtrl > MAX_Ctrl) nowCtrl = MAX_Ctrl;
    if (nowCtrl < MIN_Ctrl) nowCtrl = MIN_Ctrl;

    UIO_printf("%d\r\n", speed);

    PWM_Change(nowCtrl, 0);
}

int main()
{
    UIO_Init(115200);
    Encoder_Init();
    Timer_Init();
    PWM_Init();

    UIO_SetAsync(ENABLE);

    TIM_Cmd(TIM2, DISABLE);
    Timer_SetHandler(GetSendSpeed);
    tail = 0;
    sumsqr = 0;
    sum = 0;
    norm = 0;
    for(;;)
    {
        UIO_AsyncSend();
        if (UIO_hasNext())
        {
            double tKp, tKi, tKd;
            UIO_scanf("%s %lf %lf %lf", buff, &tKp, &tKi, &tKd);
            if (buff[1] == 'T') //START
            {
                if ((buff[2] == 'A') && (state != 1))
                {
                    sigmaError = 0;
                    TIM_Cmd(TIM2, ENABLE);
                    nowCtrl = lrint(65535 * (TargetSpeed / FullSpeed));
                    PWM_Change(nowCtrl, 0);
                    Encoder_Start();
                    state = 1;
                }
                else if ((buff[2] == 'O') && (state != 0)) //STOP
                {
                    PWM_Stop();
                    Encoder_Stop();
                    tail = 0;
                    sumsqr = 0;
                    sum = 0;
                    norm = 0;
                    memset(svspd, 0, sizeof(svspd));
                    TIM_Cmd(TIM2, DISABLE);
                    state = 0;
                }
            }
            else if ((buff[1] == 'E') && (state != 1)) //SETPID
            {
                Kp = tKp;
                Ki = tKi;
                Kd = tKd;
                UIO_printf("PID Changed! Kp = %g, Ki = %g, Kd = %g\r\n", Kp, Ki, Kd);
            }
        }
        if ((state == 1) && (((tail > SAVECOUNT) && (norm <= (SAVECOUNT + 1) / 2)) || (tail >= 100)))
        {
            PWM_Stop();
            Encoder_Stop();
            tail = 0;
            sumsqr = 0;
            sum = 0;
            norm = 0;
            memset(svspd, 0, sizeof(svspd));
            TIM_Cmd(TIM2, DISABLE);
            state = 0;
        }
    }
}
