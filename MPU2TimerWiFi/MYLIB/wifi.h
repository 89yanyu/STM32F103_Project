#ifndef __89YY_WIFI__

#define __89YY_WIFI__

#include "stm32f10x.h"

#define WIFI_SSID "HUST_WIRELESS"

#define WIFI_PSWD ""

#define WIFI_SRVIP "10.11.20.191"

#define WIFI_SRVPORT 23333

u8 WiFi_Wait(char *stdstr, int timeout);

u8 WiFi_Init(void);

u8 WiFi_Connect(void);

u8 WiFi_Send(void *buff, int len);

#endif
