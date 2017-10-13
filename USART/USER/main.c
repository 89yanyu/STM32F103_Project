#include <stdarg.h>
#include <stdio.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

#define BOUND 115200

#define BUFF_LEN 256
char read_buff[BUFF_LEN * 2];
char write_buff[BUFF_LEN];
u8 eoln = 0;
u32 buff_start = 1, buff_ending = 1;

void USART1_IRQHandler(void)
{
    u16 res;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == RESET)
    {
        return;
    }
    //USART_SendData(USART1, USART_ReceiveData(USART1));
    res = USART_ReceiveData(USART1);
    if (res == '\r')
    {
        res = 0;
    }
    else if (res == '\n')
    {
        if (read_buff[buff_ending % BUFF_LEN] == 0)
        {
            eoln++;
        }
        else
        {
            buff_start = buff_ending;
        }
        return;
    }
    read_buff[buff_ending % BUFF_LEN + BUFF_LEN] = read_buff[buff_ending % BUFF_LEN] = res;
    buff_ending++;
}

u32 uscanf(const char *format, ...)
{
    va_list args;
    u32 ret = 0;

    while (eoln==0);
    va_start(args, format);
    ret = vsscanf(read_buff + buff_start % BUFF_LEN, format, args);
    va_end (args);
    eoln--;
    while (read_buff[buff_start % BUFF_LEN] != 0) buff_start++;
    buff_start++;

    return ret;
}
u32 uprintf(const char *format, ...)
{
    u32 len, i;

    va_list args;
    va_start(args, format);
    len = vsnprintf(write_buff, BUFF_LEN, format, args);
    va_end(args);
    for (i = 0; i < len; i++)
    {
        USART_SendData(USART1, write_buff[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
    }

    return len;
}

void Init()
{
    USART_InitTypeDef USART_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    USART_DeInit(USART1);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = BOUND;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART1, ENABLE);
}

int main()
{
    u32 a, b;
    Init();
    while (uscanf("%d%d", &a, &b) == 2)
    {
        uprintf("%d+%d=%d(%d)\r\n", a, b, a + b, eoln);
    }
    uprintf("Finish\r\n");
}
