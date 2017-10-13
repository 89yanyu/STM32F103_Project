/**************************************
* 本文件为PC串口相关函数
* 本文件主要用于通过PC串口发送调试信息
* 一般是可见字符的传输，如果存在不可见字符请参照蓝牙的传输方法
**************************************/
#include "stdarg.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "usart.h"

//串口初始化
void UIO_Init(u32 bound)
{
    USART_InitTypeDef USART_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    //初始化时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    //去初始化串口
    USART_DeInit(USART1);

    //初始化IO口
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //初始化串口
    USART_InitStruct.USART_BaudRate = bound;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);

    //初始化中断
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //打开中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    //启用串口
    USART_Cmd(USART1, ENABLE);
}

//缓存区长度
#define BUFF_LEN 512
//读入缓存区，要开两倍长度
char read_buff[BUFF_LEN * 2];
//输出临时区
char write_temp[BUFF_LEN];
//输出缓冲区
char write_buff[BUFF_LEN];
//当前读入行数
u8 eoln = 0;
//读入队列的队头和队尾
u32 read_head = 1, read_tail = 1;
//输出队列的队头和队尾
u32 write_head = 1, write_tail = 1;
//是否启用异步输出
u8 async_enable = 0;

//异步输出，即不在函数完成前发送完数据，而是会持续一段时间
//而调用者也不关心何时才能发送完
//优点是调用者不需要占用比较多的时间就能完成操作
//此处的异步输出需要在空闲时调用UIO_AsyncSend，才能发送

//修改启用状态
void UIO_SetAsync(u8 isEnable)
{
    if ((async_enable != DISABLE) && (isEnable == DISABLE))
    {
        //如果此时是由异步输出转为同步输出，需要将没有输出的立即输出
        while (write_head < write_tail)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);

            USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
            write_head++;
        }
    }
    //保存启用状态
    async_enable = isEnable;
}

//串口中断
void USART1_IRQHandler(void)
{
    u16 res;
    res = USART_ReceiveData(USART1);
    //以下判断是为了同时兼容\r,\n,\r\n三种换行符
    if (res == '\r')
    {
        //收到\r，行数加一
        eoln++;
        res = 0;
    }
    else if (res == '\n')
    {
        //收到\n
        if (read_buff[(read_tail - 1) % BUFF_LEN] == 0)
        {
            //已经收到\r，忽略该字符
            return;
        }
        else
        {
            //未收到\r，行数加一
            eoln++;
            res = 0;
        }
    }
    //保存两份
    //由于缓存区采用循环队列，可能存在某一行被截成两端
    //如果保存两份则不会有这样的问题，第一段的尾部会自动延伸到第二段的头部
    //原有的一行就不会被截断
    read_buff[read_tail % BUFF_LEN + BUFF_LEN] = read_buff[read_tail % BUFF_LEN] = res;
    read_tail++;
}

//判断当前是否有可读数据
u8 UIO_hasNext()
{
    //UIO_printf("%d %d %d\r\n", read_head, read_tail, eoln);
    return (eoln > 0);
}

//获取一行
u32 UIO_gets(char *buff)
{
    int i = 0;
    //如果此时没有数据，则等待
    while (eoln == 0);
    //一直复制知道遇到\0
    //最好不要用memcpy或者strcpy，因为一行可能被隔断
    while (read_head <= read_tail)
    {
        buff[i] = read_buff[read_head % BUFF_LEN];
        read_head++;
        if (0 == buff[i])
        {
            eoln--;
            break;
        }
        i++;
    }
    return i;
}

//获取固定长度
u32 UIO_read(char *buff, int len)
{
    int i = 0;
    //如果此时没有数据，则等待
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
            //如果读到的数据跨行，行数应该减一
            eoln--;
            break;
        }
    }
    return i;
}

//用scanf从串口读入
u32 UIO_scanf(const char *format, ...)
{
    va_list args;
    u32 ret = 0;

    //如果此时没有数据，则等待
    while (eoln==0);
    va_start(args, format);
    //用scanf按格式读入连续的一段
    //缺点是，未读完的部分只能舍弃
    ret = vsscanf(read_buff + read_head % BUFF_LEN, format, args);
    va_end (args);
    //行数减一，队头后移
    eoln--;
    while (read_buff[read_head % BUFF_LEN] != 0) read_head++;
    read_head++;

    return ret;
}

//发送一定长度的数据
u32 UIO_send(char *buff, int len)
{
    int i;
    if (async_enable)
    {
        //当前是异步发送，则加入到缓存区
        for (i = 0; i < len; i++)
        {
            write_buff[write_tail % BUFF_LEN] = buff[i];
            write_tail++;
        }
    }
    else
    {
        //当前是同步发送，直接输出
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
    //按格式存储到临时区，然后调用UIO_send
    len = vsnprintf(write_temp, BUFF_LEN, format, args);
    va_end(args);

    return UIO_send(write_temp, len);
}

void UIO_AsyncSend()
{
    //当前未开启异步输出，直接返回
    if (!async_enable) return;
    if ((write_head < write_tail) &&
        (USART_GetFlagStatus(USART1, USART_FLAG_TC) == SET))
    {
        //如果当前有数据未发送，并且上次发送已经完成，则再发送一个数据
        //注意，此时如果上次发送未完成，为保证快速执行，则不应等待上次发完
        USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
        write_head++;
    }
}

