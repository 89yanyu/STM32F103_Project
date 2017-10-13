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

//定义最大最小控制量，这个值由PWM控制
#define MAXCONTROL 7200
#define MINCONTROL -7200

//字节序转换
//在多字节传输时，由于各个厂商CPU字节序不同，可能会发生错误
//目前大部分CPU为小端序，ARM也是小端序
//未知字节序的CPU可使用
//    int a = 1;
//    return ((char*)(&a) == 0)
//返回true为小端序，否则为大端序
//而一般规定网络中传输的字节序为大端序，所以需要转换字节序
#define ntoh(a, b, c) hton(a, b, c)
void hton(const void *sour, u8 len, void *dest)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        ((char*)dest)[len - 1 - i] = ((char*)sour)[i];
    }
}

//由于左右轮的目标速度可能不同时更新，
//比如更新左轮目标速度，进中断，中断中读取左右轮速度，
//此时左右轮的速度不为同一批数据
//解决办法是，用两个标记位和两个临时变量记录更新的值
//更新时先更新临时变量，当临时变量更新完成后的第一次获取时更新原有变量

//更新左右轮目标速度
static __inline void updateTargetSpeed(double left, double right)
{
    isUpdated = 0;
    UIO_printf("update taget speed %g %g\r\n", left, right);
    TargetSpeed_left_tmp = left;
    TargetSpeed_right_tmp = right;
    isUpdated = 3;
}

//获取左轮速度
static __inline double getTargetSpeed_left()
{
    if (isUpdated & 0x1)
    {
        //当左轮转速是第一次获取，更新原有左轮转速，并且左轮的标志位清空
        TargetSpeed_left = TargetSpeed_left_tmp;
        isUpdated ^= 0x1;
    }
    return TargetSpeed_left;
}
//获取右轮速度
static __inline double getTargetSpeed_right()
{
    if (isUpdated & 0x2)
    {
        //当右轮转速是第一次获取，更新原有右轮转速，并且右轮的标志位清空
        TargetSpeed_right = TargetSpeed_right_tmp;
        isUpdated ^= 0x2;
    }
    return TargetSpeed_right;
}

//处理丢失蓝牙连接
u8 LostBluetoothConnect()
{
    //立即停止，关闭编码器获取，关闭滚轮
    updateTargetSpeed(0, 0);
    Encoder_Stop_left();
    Encoder_Stop_right();
    isRunning = 0;
    Roller_OFF();
    UIO_printf("Lost bluetooth connect, STOP!\r\n");
    return 0;
}

//蓝牙掉线计数
u32 outtime = 0;

char outputstate[64];

void UpdateHandler()
{
    //u16 state[4];
    u8 statelen;
    //获取速度
    short speed_left = Encoder_GetSpeed_left();
    short speed_right = Encoder_GetSpeed_right();
    //计算偏差
    double error_left = getTargetSpeed_left() - speed_left;
    double error_right = getTargetSpeed_right() - speed_right;
    //计算偏差的一阶导
    double derror_left = error_left - lasterror_left;
    double derror_right = error_right - lasterror_right;
    //计算偏差的二阶导
    double dderror_left = error_left + pasterror_left - 2 * lasterror_left;
    double dderror_right = error_right + pasterror_right - 2 * lasterror_right;
    //保存本次和上次的速度
    pasterror_left = lasterror_left;
    lasterror_left = error_left;
    pasterror_right = lasterror_right;
    lasterror_right = error_right;

    if (isRunning)
    {
        //如果正在运行，蓝牙掉线计数加一
        outtime++;
        if (outtime > 100)
        {
            //如果计数达到100，触发掉线处理
            LostBluetoothConnect();
            outtime = 0;
        }
    }

    //control_left = lrint(getTargetSpeed_left() * 20. + 1e-6);
    //control_right = lrint(getTargetSpeed_right() * 20. + 1e-6);
    //计算PID
    control_left = lrint(control_left + Kp * derror_left + Ki * error_left + Kd * dderror_left);
    control_right = lrint(control_right + Kp * derror_right + Ki * error_right + Kd * dderror_right);

    //限制控制量的上下限
    if (control_left > MAXCONTROL) control_left = MAXCONTROL;
    if (control_right > MAXCONTROL) control_right = MAXCONTROL;
    if (control_left < MINCONTROL) control_left = MINCONTROL;
    if (control_right < MINCONTROL) control_right = MINCONTROL;
    //如果当前不在运行，强制为0
    if (!isRunning)
    {
        control_left = 0;
        control_right = 0;
    }

    //hton(&control_left, sizeof(control_left), state);
    //hton(&speed_left, sizeof(speed_left), state + 1);
    //hton(&control_right, sizeof(control_right), state + 2);
    //hton(&speed_right, sizeof(speed_right), state + 3);

    //修改PWM参数
    PWM_Change(control_left, control_right);

    if (isRunning)
    {
        //如果当前正在运行，发送此时状态
        //本应该发送二进制数据，但是由于未写上位机，所以发送明文数据
        statelen = sprintf(outputstate, "State: %d %d %d %d\r\n", control_left, speed_left, control_right, speed_right);
        HBUSART_SendFormat(HBUSART_CONTROL_STATE, statelen, (void*)outputstate);

        UIO_printf("State: %d %d %d %d %d\r\n", control_left, speed_left, control_right, speed_right, isRunning);
    }
}

int main()
{
    //各种初始化
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
        //空闲时发送数据
        UIO_AsyncSend();
        if (UIO_hasNext() && !isRunning)
        {
            //从串口设置PID
            //格式为SETPID Kp Ki Kd
            //支持小数输入
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
            //从蓝牙串口获取到数据
            if (hb_operator == HBUSART_CONTROL_FRONT)
            {
                //收到向前的指令，根据当前线速度设置左右轮转速，打开编码器
                updateTargetSpeed(TargetSpeed_linear, TargetSpeed_linear);
                Encoder_Start_left();
                Encoder_Start_right();
                isRunning = 1;
                UIO_printf("Go front");
            }
            else if (hb_operator == HBUSART_CONTROL_BACK)
            {
                //收到向后的指令，根据当前线速度设置左右轮转速，打开编码器
                updateTargetSpeed(-TargetSpeed_linear, -TargetSpeed_linear);
                Encoder_Start_left();
                Encoder_Start_right();
                isRunning = 1;
                UIO_printf("Go back");
            }
            else if (hb_operator == HBUSART_CONTROL_STOP)
            {
                //收到停止的指令，设置左右轮转速，关闭编码器
                updateTargetSpeed(0, 0);
                Encoder_Stop_left();
                Encoder_Stop_right();
                isRunning = 0;
                UIO_printf("Stop");
            }
            else if (hb_operator == HBUSART_CONTROL_TURNLEFT)
            {
                //收到左转的指令，根据当前线速度以及发来的速度差设置左右轮转速，打开编码器
                //速度差为2字节的short，如果为负，则为右转
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
                //收到右转的指令，根据当前线速度以及发来的速度差设置左右轮转速，打开编码器
                //速度差为2字节的short，如果为负，则为左转
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
                //收到开滚轮指令
                Roller_ON();
                UIO_printf("Roller ON");
            }
            else if (hb_operator == HBUSART_CONTROL_OFF)
            {
                //收到关滚轮指令
                Roller_OFF();
                UIO_printf("Roller OFF");
            }
            else if (hb_operator == HBUSART_CONTROL_SETSPEED)
            {
                //收到设置速度指令
                //格式为2字节的shrot，如果为负，取绝对值
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
                //收到脉搏指令
                //技术清零
                outtime = 0;
            }
            else
            {
                //非法指令
                UIO_printf("Error operater.");
            }

            //打印收到指令的调试信息
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
