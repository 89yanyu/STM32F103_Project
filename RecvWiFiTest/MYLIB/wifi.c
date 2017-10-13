#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wifi.h"
#include "usart.h"
#include "delay.h"
#include "led.h"

#define wifibufflen 1024
char sendbuff[wifibufflen];
char recvbuff[wifibufflen];
char tmpbuff[64];
u32 sendhead, sendtail, recvlen;
char *waitingmsg = 0;
s8 catchit;
char lastchr;
u8 sending;
u32 totallen;
u8 waitingstart;
u8 recvchannel = 0xff;
u32 recvdatalen = 0xffffffff;
u32 recvdatapos = 0xffffffff;

void wifi_SendCfg(char *Cfg)
{
    //UIO_printf("%s", Cfg);
    while (*Cfg != 0)
    {
        sendbuff[sendtail % wifibufflen] = *Cfg;
        sendtail++;
        Cfg++;
    }
    if (sending == 0)
    {
        sendhead++;
        sending = 1;
        USART_SendData(USART2, sendbuff[(sendhead - 1) % wifibufflen]);
    }
}

void wifi_SendData(void *data, int len)
{
    int i = 0;
    char *ptr = data;
    for (; i < len; i++)
    {
        sendbuff[sendtail % wifibufflen] = ptr[i];
        sendtail++;
    }
    if (sending == 0)
    {
        sendhead++;
        sending = 1;
        USART_SendData(USART2, sendbuff[(sendhead - 1) % wifibufflen]);
    }
}

void wifi_ProcRecvData(char *recv, int channel, int len)
{
    UIO_send(recv, len);
    return;
}

void wifi_ProcMsg(char *recv, int len)
{
    //UIO_printf("%s\r\n", recv);
    if (0 == waitingmsg)
    {
        return;
    }
    if (strlen(waitingmsg) != len)
    {
        return;
    }
    if (strcmp(recv, waitingmsg) == 0)
    {
        catchit = 1;
    }
    return;
}

void wifi_ProcOneChar(char nowchr)
{
    if ((recvdatalen == 0xffffffff) && ('\n' == nowchr) && ('\r' == lastchr))
    {
        return;
    }
    if ((recvdatalen == 0xffffffff) && (('\n' == nowchr) || ('\r' == nowchr)))
    {
        recvbuff[recvlen] = 0;
        wifi_ProcMsg(recvbuff, recvlen);
        recvlen = 0;
        return;
    }

    recvbuff[recvlen++] = nowchr;
    lastchr = nowchr;

    if ((recvlen == 5) && (nowchr == ','))
    {
        if (memcmp(recvbuff, "+IPD,", strlen("+IPD,")) == 0)
        {
            recvchannel = 0;
        }
    }
    if ((recvchannel != 0xff) && (nowchr == ':'))
    {
        recvbuff[recvlen - 1] = 0;
        recvdatalen = atoi(recvbuff + 5);
        recvdatapos = recvlen;
        recvbuff[recvlen - 1] = ':';
    }
    if ((recvdatalen != 0xffffffff) && (recvdatapos + recvdatalen == recvlen))
    {
        wifi_ProcRecvData(recvbuff + recvdatapos, recvchannel, recvdatalen);
        recvchannel = 0xff;
        recvdatalen = 0xffffffff;
        recvdatapos = 0xffffffff;
        recvlen = 0;
    }
}

void USART2_IRQHandler(void)
{
    if (SET == USART_GetITStatus(USART2, USART_IT_TC))
    {
        if (sendhead != sendtail)
        {
            USART_SendData(USART2, sendbuff[sendhead % wifibufflen]);
            sendhead++;
        }
        else
        {
            sending = 0;
        }
        USART_ClearITPendingBit(USART2, USART_IT_TC);
    }
    if (SET == USART_GetITStatus(USART2, USART_IT_RXNE))
    {
        wifi_ProcOneChar(USART_ReceiveData(USART2));
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

u8 WiFi_Wait(char *stdstr, int timeout)
{
    int i = 1;
    SysTick_Type *Tick = SysTick;
    u32 ms = 8000, temp;
    ms *= 10;
    catchit = 0;
    waitingmsg = stdstr;
    if (timeout < 0)
    {
        return 0;
    }
    for (i = 1; i <= timeout * 100; i++)
    {
        Tick->LOAD = ms;
        Tick->VAL = 0;
        Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

        do {
            UIO_AsyncSend();
            if (catchit != 0)
            {
                Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
                Tick->VAL = 0;
                return (catchit < 0);
            }
            temp = Tick->CTRL;
        } while ((temp & SysTick_CTRL_ENABLE_Msk) &&
                 !(temp & SysTick_CTRL_COUNTFLAG_Msk));
    }

    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    Tick->VAL = 0;

    return 1;
}

u8 WiFi_Init(void)
{
    USART_InitTypeDef USART_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    USART_DeInit(USART2);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);

    USART_Cmd(USART2, ENABLE);

    sendhead = 1;
    sendtail = 1;
    recvlen = 0;
    lastchr = 0;
    sending = 0;
    recvchannel = 0xff;
    recvdatalen = 0xffffffff;
    recvdatapos = 0xffffffff;

    wifi_SendCfg("AT+RST\r\n");
    LED_OFF(LED_0);
    if (WiFi_Wait("ready", 10)) return 1;

    wifi_SendCfg("AT+CWMODE_CUR=1\r\n");
    if (WiFi_Wait("OK", 3)) return 1;

    sprintf(tmpbuff, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PSWD);
    wifi_SendCfg(tmpbuff);
    if (WiFi_Wait("OK", 10)) return 1;

    return 0;
}

u8 WiFi_Connect(void)
{
    sprintf(tmpbuff, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", WIFI_SRVIP, WIFI_SRVPORT);
    wifi_SendCfg(tmpbuff);
    if (WiFi_Wait("OK", 3)) return 1;
    return 0;
}

u8 WiFi_Send(void *buff, int len)
{
    sprintf(tmpbuff, "AT+CIPSEND=%d\r\n", len);
    wifi_SendCfg(tmpbuff);
    if (WiFi_Wait("OK", 5)) return 1;
    wifi_SendData((char*)buff, len);
    if (WiFi_Wait("SEND OK", -1)) return 1;
    return 0;
}

u8 WiFi_SendFinish(void)
{
    return ((strcmp("SEND OK", waitingmsg) != 0) || (catchit > 0));
}
