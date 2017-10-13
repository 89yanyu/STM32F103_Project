#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <tgmath.h>

#include "bluetooth.h"
#include "delay.h"
#include "usart.h"
#include "pwm.h"
#include "encoder.h"
#include "encoder2.h"
#include "timer.h"
#include "roller.h"

HBUSART_CONTROL_TYP hb_operator;
u8 hb_len;
u8 hb_data[256];

double Kp, Ki, Kd;
double TargetSpeed_linear;
double TargetSpeed_left, TargetSpeed_right;
double TargetSpeed_left_tmp, TargetSpeed_right_tmp;
u8 isUpdated = 1;
u8 isRunning = 0;
short control_left, control_right;
short lasterror_left = 0, pasterror_left = 0;
short lasterror_right = 0, pasterror_right = 0;

//���������С�����������ֵ��PWM����
#define MAXCONTROL 7200
#define MINCONTROL -7200

//�ֽ���ת��
//�ڶ��ֽڴ���ʱ�����ڸ�������CPU�ֽ���ͬ�����ܻᷢ������
//Ŀǰ�󲿷�CPUΪС����ARMҲ��С����
//δ֪�ֽ����CPU��ʹ��
//    int a = 1;
//    return ((char*)(&a) == 0)
//����trueΪС���򣬷���Ϊ�����
//��һ��涨�����д�����ֽ���Ϊ�����������Ҫת���ֽ���
#define ntoh(a, b, c) hton(a, b, c)
void hton(const void *sour, u8 len, void *dest)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        ((char*)dest)[len - 1 - i] = ((char*)sour)[i];
    }
}

//���������ֵ�Ŀ���ٶȿ��ܲ�ͬʱ���£�
//�����������Ŀ���ٶȣ����жϣ��ж��ж�ȡ�������ٶȣ�
//��ʱ�����ֵ��ٶȲ�Ϊͬһ������
//����취�ǣ����������λ��������ʱ������¼���µ�ֵ
//����ʱ�ȸ�����ʱ����������ʱ����������ɺ�ĵ�һ�λ�ȡʱ����ԭ�б���

//����������Ŀ���ٶ�
static __inline void updateTargetSpeed(double left, double right)
{
    isUpdated = 0;
    UIO_printf("update taget speed %g %g\r\n", left, right);
    TargetSpeed_left_tmp = left;
    TargetSpeed_right_tmp = right;
    isUpdated = 3;
}

//��ȡ�����ٶ�
static __inline double getTargetSpeed_left()
{
    if (isUpdated & 0x1)
    {
        //������ת���ǵ�һ�λ�ȡ������ԭ������ת�٣��������ֵı�־λ���
        TargetSpeed_left = TargetSpeed_left_tmp;
        isUpdated ^= 0x1;
    }
    return TargetSpeed_left;
}
//��ȡ�����ٶ�
static __inline double getTargetSpeed_right()
{
    if (isUpdated & 0x2)
    {
        //������ת���ǵ�һ�λ�ȡ������ԭ������ת�٣��������ֵı�־λ���
        TargetSpeed_right = TargetSpeed_right_tmp;
        isUpdated ^= 0x2;
    }
    return TargetSpeed_right;
}

//����ʧ��������
u8 LostBluetoothConnect()
{
    //����ֹͣ���رձ�������ȡ���رչ���
    updateTargetSpeed(0, 0);
    Encoder_Stop_left();
    Encoder_Stop_right();
    isRunning = 0;
    Roller_OFF();
    UIO_printf("Lost bluetooth connect, STOP!\r\n");
    return 0;
}

//�������߼���
u32 outtime = 0;

char outputstate[64];

void UpdateHandler()
{
    //u16 state[4];
    u8 statelen;
    //��ȡ�ٶ�
    short speed_left = Encoder_GetSpeed_left();
    short speed_right = Encoder_GetSpeed_right();
    //����ƫ��
    double error_left = getTargetSpeed_left() - speed_left;
    double error_right = getTargetSpeed_right() - speed_right;
    //����ƫ���һ�׵�
    double derror_left = error_left - lasterror_left;
    double derror_right = error_right - lasterror_right;
    //����ƫ��Ķ��׵�
    double dderror_left = error_left + pasterror_left - 2 * lasterror_left;
    double dderror_right = error_right + pasterror_right - 2 * lasterror_right;
    //���汾�κ��ϴε��ٶ�
    pasterror_left = lasterror_left;
    lasterror_left = error_left;
    pasterror_right = lasterror_right;
    lasterror_right = error_right;

    if (isRunning)
    {
        //����������У��������߼�����һ
        outtime++;
        if (outtime > 100)
        {
            //��������ﵽ100���������ߴ���
            LostBluetoothConnect();
            outtime = 0;
        }
    }

    //control_left = lrint(getTargetSpeed_left() * 20. + 1e-6);
    //control_right = lrint(getTargetSpeed_right() * 20. + 1e-6);
    //����PID
    control_left = lrint(control_left + Kp * derror_left + Ki * error_left + Kd * dderror_left);
    control_right = lrint(control_right + Kp * derror_right + Ki * error_right + Kd * dderror_right);

    //���ƿ�������������
    if (control_left > MAXCONTROL) control_left = MAXCONTROL;
    if (control_right > MAXCONTROL) control_right = MAXCONTROL;
    if (control_left < MINCONTROL) control_left = MINCONTROL;
    if (control_right < MINCONTROL) control_right = MINCONTROL;
    //�����ǰ�������У�ǿ��Ϊ0
    if (!isRunning)
    {
        control_left = 0;
        control_right = 0;
    }

    //hton(&control_left, sizeof(control_left), state);
    //hton(&speed_left, sizeof(speed_left), state + 1);
    //hton(&control_right, sizeof(control_right), state + 2);
    //hton(&speed_right, sizeof(speed_right), state + 3);

    //�޸�PWM����
    PWM_Change(control_left, control_right);

    if (isRunning)
    {
        //�����ǰ�������У����ʹ�ʱ״̬
        //��Ӧ�÷��Ͷ��������ݣ���������δд��λ�������Է�����������
        statelen = sprintf(outputstate, "State: %d %d %d %d\r\n", control_left, speed_left, control_right, speed_right);
        HBUSART_SendFormat(HBUSART_CONTROL_STATE, statelen, (void*)outputstate);

        UIO_printf("State: %d %d %d %d %d\r\n", control_left, speed_left, control_right, speed_right, isRunning);
    }
}

int main()
{
    //���ֳ�ʼ��
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    HBUSART_MyInit();
    UIO_Init(115200);
    Delay_Init();
    PWM_Init();
    Roller_Init();

    Encoder_Init_left();
    Encoder_Init_right();

    Timer_Init();
    Timer_SetHandler(UpdateHandler);

    UIO_printf("GOGOGO!!\r\n");
    UIO_SetAsync(ENABLE);

    while (1)
    {
        //����ʱ��������
        UIO_AsyncSend();
        if (UIO_hasNext() && !isRunning)
        {
            //�Ӵ�������PID
            //��ʽΪSETPID Kp Ki Kd
            //֧��С������
            double p, i, d;
            char cmd[8];
            int ret;
            ret = UIO_scanf("%s %lf %lf %lf", cmd, &p, &i, &d);
            if ((ret == 4) && (strcmp(cmd, "SETPID") == 0))
            {
                Kp = p;
                Ki = i;
                Kd = d;
                UIO_printf("Set PID successed %g %g %g\r\n", Kp, Ki, Kd);
            }
        }
        hb_len = HBUSART_GetData(&hb_operator, hb_data);
        if (hb_operator < HBUSART_CONTROL_MAX)
        {
            //���������ڻ�ȡ������
            if (hb_operator == HBUSART_CONTROL_FRONT)
            {
                //�յ���ǰ��ָ����ݵ�ǰ���ٶ�����������ת�٣��򿪱�����
                updateTargetSpeed(TargetSpeed_linear, TargetSpeed_linear);
                Encoder_Start_left();
                Encoder_Start_right();
                isRunning = 1;
                UIO_printf("Go front");
            }
            else if (hb_operator == HBUSART_CONTROL_BACK)
            {
                //�յ�����ָ����ݵ�ǰ���ٶ�����������ת�٣��򿪱�����
                updateTargetSpeed(-TargetSpeed_linear, -TargetSpeed_linear);
                Encoder_Start_left();
                Encoder_Start_right();
                isRunning = 1;
                UIO_printf("Go back");
            }
            else if (hb_operator == HBUSART_CONTROL_STOP)
            {
                //�յ�ֹͣ��ָ�����������ת�٣��رձ�����
                updateTargetSpeed(0, 0);
                Encoder_Stop_left();
                Encoder_Stop_right();
                isRunning = 0;
                UIO_printf("Stop");
            }
            else if (hb_operator == HBUSART_CONTROL_TURNLEFT)
            {
                //�յ���ת��ָ����ݵ�ǰ���ٶ��Լ��������ٶȲ�����������ת�٣��򿪱�����
                //�ٶȲ�Ϊ2�ֽڵ�short�����Ϊ������Ϊ��ת
                char tmp;
                double diffSpeed;
                hton(hb_data, sizeof(char), &tmp);
                diffSpeed = fabs(tmp);
                if (diffSpeed >= TargetSpeed_linear * 2)
                {
                    diffSpeed = TargetSpeed_linear;
                }
                else
                {
                    diffSpeed /= 2;
                }
                updateTargetSpeed(TargetSpeed_linear - diffSpeed, TargetSpeed_linear + diffSpeed);
                Encoder_Start_left();
                Encoder_Start_right();
                isRunning = 1;
                UIO_printf("Turn left");
            }
            else if (hb_operator == HBUSART_CONTROL_TURNRIGHT)
            {
                //�յ���ת��ָ����ݵ�ǰ���ٶ��Լ��������ٶȲ�����������ת�٣��򿪱�����
                //�ٶȲ�Ϊ2�ֽڵ�short�����Ϊ������Ϊ��ת
                char tmp;
                double diffSpeed;
                hton(hb_data, sizeof(char), &tmp);
                diffSpeed = fabs(tmp);
                if (diffSpeed >= TargetSpeed_linear * 2)
                {
                    diffSpeed = TargetSpeed_linear;
                }
                else
                {
                    diffSpeed /= 2;
                }
                updateTargetSpeed(TargetSpeed_linear + diffSpeed, TargetSpeed_linear - diffSpeed);
                Encoder_Start_left();
                Encoder_Start_right();
                isRunning = 1;
                UIO_printf("Turn right");
            }
            else if (hb_operator == HBUSART_CONTROL_ON)
            {
                //�յ�������ָ��
                Roller_ON();
                UIO_printf("Roller ON");
            }
            else if (hb_operator == HBUSART_CONTROL_OFF)
            {
                //�յ��ع���ָ��
                Roller_OFF();
                UIO_printf("Roller OFF");
            }
            else if (hb_operator == HBUSART_CONTROL_SETSPEED)
            {
                //�յ������ٶ�ָ��
                //��ʽΪ2�ֽڵ�shrot�����Ϊ����ȡ����ֵ
                short tmp;
                hton(hb_data, sizeof(short), &tmp);
                TargetSpeed_linear = fabs(tmp * 1.0);
                UIO_printf("Set speed");
            }
            else if (hb_operator == HBUSART_CONTROL_SETPID)
            {
                UIO_printf("Set PID");
            }
            else if (hb_operator == HBUSART_CONTROL_ALIVE)
            {
                //�յ�����ָ��
                //��������
                outtime = 0;
            }
            else
            {
                //�Ƿ�ָ��
                UIO_printf("Error operater.");
            }

            //��ӡ�յ�ָ��ĵ�����Ϣ
            if (hb_operator != HBUSART_CONTROL_ALIVE)
            {
                if (hb_operator < HBUSART_CONTROL_MAX)
                {
                    u8 i;
                    for (i = 0; i < hb_len; i++)
                    {
                        UIO_printf(" 0x%02x", (hb_data[i] & 0xff));
                    }
                }
                UIO_printf("\r\n");
            }
        }
    }
}
