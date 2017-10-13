/**************************************
* ���ļ�ΪPC������غ���
* ���ļ���Ҫ����ͨ��PC���ڷ��͵�����Ϣ
* һ���ǿɼ��ַ��Ĵ��䣬������ڲ��ɼ��ַ�����������Ĵ��䷽��
**************************************/
#include "stdarg.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "usart.h"

//���ڳ�ʼ��
void UIO_Init(u32 bound)
{
    USART_InitTypeDef USART_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    //��ʼ��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    //ȥ��ʼ������
    USART_DeInit(USART1);

    //��ʼ��IO��
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //��ʼ������
    USART_InitStruct.USART_BaudRate = bound;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);

    //��ʼ���ж�
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //���ж�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    //���ô���
    USART_Cmd(USART1, ENABLE);
}

//����������
#define BUFF_LEN 512
//���뻺������Ҫ����������
char read_buff[BUFF_LEN * 2];
//�����ʱ��
char write_temp[BUFF_LEN];
//���������
char write_buff[BUFF_LEN];
//��ǰ��������
u8 eoln = 0;
//������еĶ�ͷ�Ͷ�β
u32 read_head = 1, read_tail = 1;
//������еĶ�ͷ�Ͷ�β
u32 write_head = 1, write_tail = 1;
//�Ƿ������첽���
u8 async_enable = 0;

//�첽����������ں������ǰ���������ݣ����ǻ����һ��ʱ��
//��������Ҳ�����ĺ�ʱ���ܷ�����
//�ŵ��ǵ����߲���Ҫռ�ñȽ϶��ʱ�������ɲ���
//�˴����첽�����Ҫ�ڿ���ʱ����UIO_AsyncSend�����ܷ���

//�޸�����״̬
void UIO_SetAsync(u8 isEnable)
{
    if ((async_enable != DISABLE) && (isEnable == DISABLE))
    {
        //�����ʱ�����첽���תΪͬ���������Ҫ��û��������������
        while (write_head < write_tail)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);

            USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
            write_head++;
        }
    }
    //��������״̬
    async_enable = isEnable;
}

//�����ж�
void USART1_IRQHandler(void)
{
    u16 res;
    res = USART_ReceiveData(USART1);
    //�����ж���Ϊ��ͬʱ����\r,\n,\r\n���ֻ��з�
    if (res == '\r')
    {
        //�յ�\r��������һ
        eoln++;
        res = 0;
    }
    else if (res == '\n')
    {
        //�յ�\n
        if (read_buff[(read_tail - 1) % BUFF_LEN] == 0)
        {
            //�Ѿ��յ�\r�����Ը��ַ�
            return;
        }
        else
        {
            //δ�յ�\r��������һ
            eoln++;
            res = 0;
        }
    }
    //��������
    //���ڻ���������ѭ�����У����ܴ���ĳһ�б��س�����
    //������������򲻻������������⣬��һ�ε�β�����Զ����쵽�ڶ��ε�ͷ��
    //ԭ�е�һ�оͲ��ᱻ�ض�
    read_buff[read_tail % BUFF_LEN + BUFF_LEN] = read_buff[read_tail % BUFF_LEN] = res;
    read_tail++;
}

//�жϵ�ǰ�Ƿ��пɶ�����
u8 UIO_hasNext()
{
    //UIO_printf("%d %d %d\r\n", read_head, read_tail, eoln);
    return (eoln > 0);
}

//��ȡһ��
u32 UIO_gets(char *buff)
{
    int i = 0;
    //�����ʱû�����ݣ���ȴ�
    while (eoln == 0);
    //һֱ����֪������\0
    //��ò�Ҫ��memcpy����strcpy����Ϊһ�п��ܱ�����
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

//��ȡ�̶�����
u32 UIO_read(char *buff, int len)
{
    int i = 0;
    //�����ʱû�����ݣ���ȴ�
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
            //������������ݿ��У�����Ӧ�ü�һ
            eoln--;
            break;
        }
    }
    return i;
}

//��scanf�Ӵ��ڶ���
u32 UIO_scanf(const char *format, ...)
{
    va_list args;
    u32 ret = 0;

    //�����ʱû�����ݣ���ȴ�
    while (eoln==0);
    va_start(args, format);
    //��scanf����ʽ����������һ��
    //ȱ���ǣ�δ����Ĳ���ֻ������
    ret = vsscanf(read_buff + read_head % BUFF_LEN, format, args);
    va_end (args);
    //������һ����ͷ����
    eoln--;
    while (read_buff[read_head % BUFF_LEN] != 0) read_head++;
    read_head++;

    return ret;
}

//����һ�����ȵ�����
u32 UIO_send(char *buff, int len)
{
    int i;
    if (async_enable)
    {
        //��ǰ���첽���ͣ�����뵽������
        for (i = 0; i < len; i++)
        {
            write_buff[write_tail % BUFF_LEN] = buff[i];
            write_tail++;
        }
    }
    else
    {
        //��ǰ��ͬ�����ͣ�ֱ�����
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
    //����ʽ�洢����ʱ����Ȼ�����UIO_send
    len = vsnprintf(write_temp, BUFF_LEN, format, args);
    va_end(args);

    return UIO_send(write_temp, len);
}

void UIO_AsyncSend()
{
    //��ǰδ�����첽�����ֱ�ӷ���
    if (!async_enable) return;
    if ((write_head < write_tail) &&
        (USART_GetFlagStatus(USART1, USART_FLAG_TC) == SET))
    {
        //�����ǰ������δ���ͣ������ϴη����Ѿ���ɣ����ٷ���һ������
        //ע�⣬��ʱ����ϴη���δ��ɣ�Ϊ��֤����ִ�У���Ӧ�ȴ��ϴη���
        USART_SendData(USART1, write_buff[write_head % BUFF_LEN]);
        write_head++;
    }
}

