#include "stdarg.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "usart.h"

void UIO_Init(u32 bound)
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

    USART_InitStruct.USART_BaudRate = bound;
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

#define BUFF_LEN 512
char read_buff[BUFF_LEN * 2];
char write_temp[BUFF_LEN];
char write_buff[BUFF_LEN];
u8 eoln = 0;
u32 read_head = 1, read_tail = 1;
u32 write_head = 1, write_tail = 1;
u8 async_enable = 0;

void UIO_SetAsync(u8 isEnable)
{
    async_enable = isEnable;
}

void USART1_IRQHandler(void)
{
    u16 res;
    res = USART_ReceiveData(USART1);
    if (res == '\r')
    {
        eoln++;
        res = 0;
    }
    else if (res == '\n')
    {
        if (read_buff[(read_tail - 1) % BUFF_LEN] == 0)
        {
            return;
        }
        else
        {
            eoln++;
            res = 0;
        }
    }
    read_buff[read_tail % BUFF_LEN + BUFF_LEN] = read_buff[read_tail % BUFF_LEN] = res;
    read_tail++;
}

u8 UIO_hasNext()
{
    //UIO_printf("%d %d %d\r\n", read_head, read_tail, eoln);
    return (eoln > 0);
}

u32 UIO_read(char *buff, int len)
{
    int i = 0;
    while (eoln == 0);
    for (i = 0; i < len; i++)
    {
        if (read_head >= read_tail)
        {
            break;
        }
        buff[i] = read_buff[read_head % BUFF_LEN];
        read_head++;
        if (0 == buff[i])
        {
            eoln--;
            break;
        }
    }
    return i;
}

u32 UIO_scanf(const char *format, ...)
{
    va_list args;
    u32 ret = 0;

    while (eoln==0);
    va_start(args, format);
    ret = vsscanf(read_buff + read_head % BUFF_LEN, format, args);
    va_end (args);
    eoln--;
    while (read_buff[read_head % BUFF_LEN] != 0) read_head++;
    read_head++;

    return ret;
}

u32 UIO_send(char *buff, int len)
{
    int i;
    if (async_enable)
    {
        while (write_tail - write_head + len > BUFF_LEN)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
            USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
            write_head++;
        }
        if (write_tail > write_head)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
            USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
            write_head++;
        }
        for (i = 0; i < len; i++)
        {
            write_buff[write_tail % BUFF_LEN] = buff[i];
            write_tail++;
        }
    }
    else
    {
        for (i = 0; i < len; i++)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
            USART_SendData(USART1, buff[i]);
        }
    }

    return len;
}

u32 UIO_printf(const char *format, ...)
{
    u32 len;

    va_list args;
    va_start(args, format);
    len = vsnprintf(write_temp, BUFF_LEN, format, args);
    va_end(args);

    return UIO_send(write_temp, len);
}

void UIO_AsyncSend()
{
    if (!async_enable) return;
    if ((write_head < write_tail) &&
        (USART_GetFlagStatus(USART1, USART_FLAG_TC) == SET))
    {
        USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
        write_head++;
    }
}

