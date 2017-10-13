#ifndef __89YY_WIFI__

#define __89YY_WIFI__

#include "stm32f10x.h"

#define WIFI_SSID "HUST_WIRELESS"//"HUST@121"//

#define WIFI_PSWD ""//"HUST2014@YY121"//

#define WIFI_SRVIP "10.11.22.154"//"192.168.1.100"//

#define WIFI_SRVPORT 23333

u8 WiFi_Wait(char *stdstr, int timeout);

u8 WiFi_Init(void);

u8 WiFi_Connect(void);

u8 WiFi_Send(void *buff, int len);

u8 WiFi_SendFinish(void);

u32 WiFi_Read(char *buff);

u8 WiFi_isClosed(void);

#endif
