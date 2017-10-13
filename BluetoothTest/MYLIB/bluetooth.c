/**********************************
* ���ļ�Ϊ��������ͨ�š�
* ���а����˴�CRCУ���ͨ��Э�飬��
* �Ա�֤�������ݵ���ȷ�ԡ�
**********************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bluetooth.h"
#include "delay.h"
#include "usart.h"

/**********************************
* ͨ��Э��ĸ�ʽΪ:
* +--------+--------+--------+----------------+--------+--------+
* | ��ʼ֡ | ����֡ | ����֡ |     ����֡     | У��֡ | ����֡ |
* +--------+--------+--------+----------------+--------+--------+
*  1 byte   1 byte   1 byte   n byte           2 byte   1 byte
* ��ʼ֡�̶�Ϊ0x02
* ����֡�̶�Ϊ0x03
* ����֡��HBUSART_CONTROL_TYP����
* ����֡Ϊ�������ݵĳ���
* ����֡Ϊ���η��͵�����
* У��֡ΪCRC-CCITT(0xFFFF)У��ֵ��У������Ϊ����֡+����֡+����֡
* �����Ҫ�ֶ�����У��֡�ɷ��� https://www.lammertbies.nl/comm/info/crc-calculation.html
* ȷ��CRCУ��ɹ����Խ�У��֡����У�飬��У�����ݸ�Ϊ����֡+����֡+����֡+У��֡
* ����У��ֵһ��Ϊ0����Ϊ0��У��ʧ��
**********************************/

//���շ��ͻ���������󳤶�
#define HBUSART_BUFFLEN 2048

//����״̬����
typedef enum {
  //��ʼ״̬���ȴ����տ�ʼ֡
  HBUSART_STATE_READY = 0,
  //�ȴ���������֡
  HBUSART_STATE_WAITTYPE,
  //�ȴ����ճ���֡
  HBUSART_STATE_WAITLENGTH,
  //�ȴ���������֡
  HBUSART_STATE_WAITDATA,
  //�ȴ�����У��֡�ĸ��ֽ�
  HBUSART_STATE_WAITCRCHIGH,
  //�ȴ�����У��֡�ĵ��ֽ�
  HBUSART_STATE_WAITCRCLOW,
  //�ȴ����ս���֡
  HBUSART_STATE_WAITTAIL,
  //��Ч״̬
  HBUSART_STATE_MAX
} HBUSART_STATE_TYP;

//��ǰ����״̬
HBUSART_STATE_TYP HBUSART_State = HBUSART_STATE_READY;

//���ν��յĳ���
u8 HBUSART_DataLen = 0;

//���ͻ�����
u8 HBUSART_SendBuff[HBUSART_BUFFLEN];

//���ջ�����
u8 HBUSART_RecvBuff[HBUSART_BUFFLEN];

//���ͻ�����ѭ�����ж�ͷ
u32 HBUSART_SendHead;

//���ͻ�����ѭ�����ж�β
u32 HBUSART_SendTail;

//���ܻ������ĳ���
u32 HBUSART_RecvLen;

//��ǰ�Ƿ����ڷ���
u8 HBUSART_isSending;

//��ǰ��У��ֵ����Ҫ�Ǳ����м�׶�
u16 HBUSART_CRC;

//���ν��յ��Ĳ���
HBUSART_CONTROL_TYP HBUSART_Operate = HBUSART_CONTROL_MAX;

//�����Ѿ����յ��ĳ���
u16 HBUSART_WaitLen;

//CRC�����
static u16 ccitt_table[256] = {
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

//�ۻ�����CRC
static u16 hbusart_CalcCRC16(u16 crc, u8 *q, int len)
{
    while (len-- > 0)
        crc = ccitt_table[(crc >> 8 ^ *q++) & 0xff] ^ (crc << 8);
    return crc;
}

//����ʽ����
void HBUSART_SendFormat(u8 control, u8 len, u8 *ptr)
{
    //��ʼ��crcΪ0xffff
    u16 crc = 0xffff;
    u16 i;

    //���Ʒ��ͳ���
    len &= 0xFF;
    //���Ʋ�������
    control &= 0xFF;

    //���ڵ����в�Ϊ��ʱ��һֱ���ͣ�����ֻ��Ҫ�����͵�����ѹ����м���

    //������ʼ֡0x02
    HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = 0x02;
    //��������֡
    HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = control;
    //��������֡��crc
    crc = hbusart_CalcCRC16(crc, &control, 1);
    //���س���֡
    HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = len;
    //�ۼӳ���֡��crc
    crc = hbusart_CalcCRC16(crc, &len, 1);
    //��������֡
    for (i = 0; i < len; i++)
    {
        HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = ptr[i];
    }
    //�ۼӳ���֡��crc
    crc = hbusart_CalcCRC16(crc, ptr, len);
    //����У��֡
    HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = crc >> 8;
    HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = crc & 0xff;
    //У�����֡
    HBUSART_SendBuff[HBUSART_SendTail++ % HBUSART_BUFFLEN] = 0x03;

    //�����ڷ��͵�ʱ�򣬼�ʹѹ�������Ҳ���ᷢ�ͣ�������Ҫ�ֶ���������
    if (HBUSART_isSending == 0)
    {
        HBUSART_isSending = 1;
        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
    }
}

//������յ��ĵ����ַ�����Ҫ��״̬��ת��
void hbusart_ProcOneChar(u8 nowchar)
{
    if (HBUSART_State == HBUSART_STATE_READY)
    {
        //��ǰ״̬Ϊ׼��״̬
        if (nowchar != 0x02)
        {
            //�յ�����Ŀ�ʼ֡������NAK
            HBUSART_SendFormat(HBUSART_CONTROL_NAK, 18, (void*)"Error start frame.");
        }
        else
        {
            //�յ���ȷ�Ŀ�ʼ֡���޸�״̬
            HBUSART_State = HBUSART_STATE_WAITTYPE;
            //��ʼ��crc
            HBUSART_CRC = 0xffff;
        }
    }
    else if (HBUSART_State == HBUSART_STATE_WAITTYPE)
    {
        //��ǰ״̬Ϊ�ȴ�����֡
        if (nowchar >= HBUSART_CONTROL_MAX)
        {
            //�յ����������֡������NAK
            HBUSART_SendFormat(HBUSART_CONTROL_NAK, 18, (void*)"Error operator.");
            //��ԭ״̬
            HBUSART_State = HBUSART_STATE_READY;
        }
        else
        {
            //�ۼ�crc
            HBUSART_CRC = hbusart_CalcCRC16(HBUSART_CRC, &nowchar, 1);
            //�����������
            HBUSART_Operate = (HBUSART_CONTROL_TYP)nowchar;
            //�޸�״̬
            HBUSART_State = HBUSART_STATE_WAITLENGTH;
        }
    }
    else if (HBUSART_State == HBUSART_STATE_WAITLENGTH)
    {
        //��ǰ״̬Ϊ�ȴ�����֡
        //�ۼ�crc
        HBUSART_CRC = hbusart_CalcCRC16(HBUSART_CRC, &nowchar, 1);
        //�������ݳ���
        HBUSART_DataLen = nowchar;
        //��ʼ����ǰ���ճ���
        HBUSART_WaitLen = 0;
        if (HBUSART_DataLen == 0)
        {
            //�������֡����Ϊ0����ֱ��ת��������У��֡״̬
            HBUSART_State = HBUSART_STATE_WAITCRCHIGH;
        }
        else
        {
            //�޸�״̬
            HBUSART_State = HBUSART_STATE_WAITDATA;
        }
    }
    else if (HBUSART_State == HBUSART_STATE_WAITDATA)
    {
        //��������֡���ۼ�crc
        HBUSART_RecvBuff[HBUSART_WaitLen++] = nowchar;
        HBUSART_CRC = hbusart_CalcCRC16(HBUSART_CRC, &nowchar, 1);

        if (HBUSART_WaitLen >= HBUSART_DataLen)
        {
            //���������ɣ��޸�״̬
            HBUSART_State = HBUSART_STATE_WAITCRCHIGH;
        }
    }

    else if (HBUSART_State == HBUSART_STATE_WAITCRCHIGH)
    {
        //�ۼ�crc���޸�״̬
        HBUSART_CRC = hbusart_CalcCRC16(HBUSART_CRC, &nowchar, 1);
        HBUSART_State = HBUSART_STATE_WAITCRCLOW;
    }
    else if (HBUSART_State == HBUSART_STATE_WAITCRCLOW)
    {
        //�ۼ�crc
        HBUSART_CRC = hbusart_CalcCRC16(HBUSART_CRC, &nowchar, 1);
        if (HBUSART_CRC != 0)
        {
            //���crcУ��ʧ�ܣ�����NAK
            HBUSART_SendFormat(HBUSART_CONTROL_NAK, 17, (void*)"CRC check failed.");
            //��ԭ״̬
            HBUSART_State = HBUSART_STATE_READY;
            HBUSART_Operate = HBUSART_CONTROL_MAX;
        }
        else
        {
            //�޸�״̬
            HBUSART_State = HBUSART_STATE_WAITTAIL;
        }
    }
    else if (HBUSART_State == HBUSART_STATE_WAITTAIL)
    {
        if (nowchar != 0x03)
        {
            //����յ�����Ľ���֡������NAK
            HBUSART_SendFormat(HBUSART_CONTROL_NAK, 16, (void*)"Error end frame.");
            HBUSART_Operate = HBUSART_CONTROL_MAX;
        }
        else
        {
            //�����ǰ���Ͳ�Ϊ��������������ACK
            if (HBUSART_Operate != HBUSART_CONTROL_ALIVE)
            {
                HBUSART_SendFormat(HBUSART_CONTROL_ACK, 2, (void*)"OK");
            }
        }
        //��ԭ״̬
        HBUSART_State = HBUSART_STATE_READY;
    }
}

//�����жϺ���
void USART3_IRQHandler(void)
{
    //����쳣״̬
    if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
    {
        USART_ReceiveData(USART3);
        USART_ClearFlag(USART3, USART_FLAG_ORE);
    }
    if(USART_GetFlagStatus(USART3, USART_FLAG_NE) != RESET)
    {
        USART_ClearFlag(USART3, USART_FLAG_NE);
    }
    if(USART_GetFlagStatus(USART3, USART_FLAG_FE) != RESET)
    {
        USART_ClearFlag(USART3, USART_FLAG_FE);
    }
    if(USART_GetFlagStatus(USART3, USART_FLAG_PE) != RESET)
    {
        USART_ClearFlag(USART3, USART_FLAG_PE);
    }

    //��������ж�
    if (RESET != USART_GetITStatus(USART3, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        hbusart_ProcOneChar(USART_ReceiveData(USART3));
    }
    else if (RESET != USART_GetITStatus(USART3, USART_IT_TXE))
    {
        //TX Empty��ͬ��Transmit Complete
        //TC�ǵ��ַ�����λ�Ĵ�������ȫ�Ƴ��󴥷��������ƺ�ÿ�γ�����Ҫ�ֶ���գ�
        //TXE�ǵ��ַ���DR�Ĵ������Ƶ���λ�Ĵ����󴥷������仰˵����DR�Ĵ������˾ʹ���
        //Ȼ�������պͷ����ƺ�����DR�Ĵ�������֮��᲻���и��Ż���֪��
        //��Ŀǰ������д��Ӧ����û������
        if (HBUSART_SendHead != HBUSART_SendTail)
        {
            //�����ʱ��������δ������
            USART_SendData(USART3, HBUSART_SendBuff[HBUSART_SendHead % HBUSART_BUFFLEN]);
            HBUSART_SendHead++;
        }
        else
        {
            //����Ѿ�������ɣ����жϿ��عص�����Ȼ��һֱ����
            //��֮�������Ҫ�������ͣ�ֻ�轫�ն˿��ش�
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
            HBUSART_isSending = 0;
        }
    }
}

//��ȡ��������
u8 HBUSART_GetData(HBUSART_CONTROL_TYP *op, u8 *data)
{
    u8 ret;

    *op = HBUSART_CONTROL_MAX;
    if ((HBUSART_State != HBUSART_STATE_READY) || (HBUSART_Operate == HBUSART_CONTROL_MAX))
    {
        //�����ǰû�н��յ�һ�����������ݣ�����0
        return 0;
    }

    //��������
    *op = HBUSART_Operate;
    ret = HBUSART_DataLen;
    memcpy(data, HBUSART_RecvBuff, HBUSART_DataLen);
    //�������
    HBUSART_Operate = HBUSART_CONTROL_MAX;
    HBUSART_DataLen = 0;
    //ע�⣬�����λ����������̫�죬�������ݻᱻ����
    return ret;
}

//���ڳ�ʼ��
void HBUSART_MyInit(void)
{
    USART_InitTypeDef USART_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    //��ʼ��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    //����3ȥ��ʼ��
    USART_DeInit(USART3);

    //��ʼ��IO��
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //��ʼ������3
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStruct);

    //��ʼ���жϣ�ͳһ���÷���2:2
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //�򿪽����ն�
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    //��������
    USART_Cmd(USART3, ENABLE);

    //ȫ�ֱ�����ʼ��
    HBUSART_SendHead = 1;
    HBUSART_SendTail = 1;
    HBUSART_RecvLen = 0;
    HBUSART_isSending = 0;
}
