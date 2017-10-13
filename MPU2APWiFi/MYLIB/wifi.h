#ifndef __89YY_WIFI__

#define __89YY_WIFI__

#include "stm32f10x.h"

#define WIFI_SSID "STM32WIFI"

#define WIFI_PSWD "19961104pyy"

#define WIFI_SRVIP "192.168.4.1"

#define WIFI_SRVPORT 23333

u8 WiFi_Wait(char *stdstr, int timeout);

u8 WiFi_Init(void);

u8 WiFi_ReduceConnect(void);

u8 WiFi_isAnyoneConnected(void);

u8 WiFi_Send(char *buff, int len);

#endif
