#include "stm32f10x_gpio.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "encoder.h"
#include "pwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

const unsigned char show[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};
const uint16_t GPIO_Pin_LowByte = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
const uint16_t GPIO_Pin_HighByte = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 |
                                   GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
double numshow = 0.;
u8 debuginfo[64];

#define ntoh(a, b, c) hton(a, b, c)
void hton(const void *sour, u8 len, void *dest)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        ((char*)dest)[len - 1 - i] = ((char*)sour)[i];
    }
}

void ShowNum()
{
    int bit, tmp;
    u8 isNeg = 0;
    bit = 0;
    tmp = lrint(numshow * 100 + 1e-6);
    isNeg = (tmp < 0);
    tmp = fabs(tmp);
    while ((bit <= 2) || (tmp > 0))
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_LowByte);
        GPIO_SetBits(GPIOB, GPIO_Pin_HighByte);
        GPIO_ResetBits(GPIOC, show[tmp % 10]);
        GPIO_ResetBits(GPIOB, 1 << (15 - bit));
        if (bit == 2)
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
        }
        Delay_ms_Lazy(3);
        tmp /= 10;
        bit++;
    }
    if (isNeg)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_LowByte);
        GPIO_SetBits(GPIOB, GPIO_Pin_HighByte);
        GPIO_ResetBits(GPIOC, 1 << 6);
        GPIO_ResetBits(GPIOB, 1 << (15 - bit));
        Delay_ms_Lazy(3);
    }
    GPIO_SetBits(GPIOC, GPIO_Pin_LowByte);
    GPIO_SetBits(GPIOB, GPIO_Pin_HighByte);
}

void LED8_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_LowByte;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //high
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_HighByte;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOC, GPIO_Pin_LowByte);
    GPIO_SetBits(GPIOB, GPIO_Pin_HighByte);
}

void RCC_Init()
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    if(RCC_WaitForHSEStartUp() == SUCCESS)
    {
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        FLASH_SetLatency(FLASH_Latency_2);
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while(RCC_GetSYSCLKSource() != 0x08);
    }
}

double Kp, Ki, Kd;
double TargetSpeed;
u8 isRunning = 0;
short control;
short lasterror = 0, pasterror = 0;
char tmp[64];
int tmplen;

void UpdateHandler()
{
    short speed = Encoder_GetSpeed() * 15;
    u16 state[2];
    double error = TargetSpeed - speed;
    double derror = error - lasterror;
    double dderror = error + pasterror - 2 * lasterror;
    int maxControl = 700, minControl = -700;
    pasterror = lasterror;
    lasterror = error;
    
    control = lrint(control + Kp * derror + Ki * error + Kd * dderror);
    
    if (control > maxControl) control = maxControl;
    if (control < minControl) control = minControl;
    if (!isRunning) control = 0;

    //control = lrint(TargetSpeed / 8. + 1e-6);

    hton(&control, sizeof(control), state);
    hton(&speed, sizeof(speed), state + 1);

    PWM_Change(control);

    USART_SendFormat(USART_CONTROL_STATE, 4, (void*)state);
    numshow = speed;
}

int main()
{
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    RCC_Init();
    LED8_Init();
    Delay_Init();
    USART_MyInit();
    Timer_Init();
    Timer_SetHandler(UpdateHandler);
    Encoder_Init();
    Encoder_Start();
    PWM_Init();

    USART_SendFormat(USART_CONTROL_DEBUG, 8, (void*)"GOGOGO!!");
    for (;;)
    {
        USART_CONTROL_TYP op;
        u16 len;
        u8 *data;
        ShowNum();
        if (USART_GetData(&op, &len, &data))
        {
            if (op == USART_CONTROL_SETPID)
            {
                if (isRunning)
                {
                    USART_SendFormat(USART_CONTROL_NAK, 13, (void*)"It's running.");
                }
                else if (len != sizeof(double) * 3)
                {
                    USART_SendFormat(USART_CONTROL_NAK, 18, (void*)"Error data length.");
                }
                else
                {
                    double *pid = (void*)data;
                    ntoh(pid, sizeof(double), &Kp);
                    ntoh(pid + 1, sizeof(double), &Ki);
                    ntoh(pid + 2, sizeof(double), &Kd);
                    USART_SendFormat(USART_CONTROL_ACK, 2, (void*)"OK");
                }
            }
            else if (op == USART_CONTROL_SETSPEED)
            {
                if (len != sizeof(double))
                {
                    USART_SendFormat(USART_CONTROL_NAK, 18, (void*)"Error data length.");
                }
                else
                {
                    ntoh(data, sizeof(double), &TargetSpeed);
                    USART_SendFormat(USART_CONTROL_ACK, 2, (void*)"OK");
                }
            }
            else if (op == USART_CONTROL_START)
            {
                if (isRunning)
                {
                    USART_SendFormat(USART_CONTROL_ACK, 13, (void*)"It's running.");
                }
                else
                {
                    isRunning = 1;
                    USART_SendFormat(USART_CONTROL_ACK, 2, (void*)"OK");
                }
            }
            else if (op == USART_CONTROL_STOP)
            {
                if (isRunning)
                {
                    isRunning = 0;
                    USART_SendFormat(USART_CONTROL_ACK, 2, (void*)"OK");
                }
                else
                {
                    USART_SendFormat(USART_CONTROL_ACK, 13, (void*)"It's not running.");
                }
            }
            else
            {
                u16 len = sprintf((void*)debuginfo, "Unknown operator(0x%02x).", op);
                USART_SendFormat(USART_CONTROL_NAK, len, debuginfo);
            }
        }
    }
}
