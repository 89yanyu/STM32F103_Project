#ifndef __89YY_WIFISTAGE__

#define __89YY_WIFISTAGE__

#include "wifi.h"

typedef enum
{
    WIFISTAGE_RESETTING = 0,
    WIFISTAGE_CHANGINGMODE,
    WIFISTAGE_CONNECTINGWLAN,
    WIFISTAGE_CONNECTINGSERVER,
    WIFISTAGE_READY,
    WIFISTAGE_WAITINGSENDRSP,
    WIFISTAGE_SENDING
} WIFISTAGE_TYPE;

typedef struct {
    char *command;
    WIFISTAGE_TYPE nextstage;
} WIFISTAGE_CHANGESTEP;

WIFISTAGE_TYPE Succ_Step[] = {{"AT+CWMODE_CUR=1\r\n", WIFISTAGE_CHANGINGMODE},
                              {"AT+CWJAP_CUR=\""WIFI_SSID"\",\""WIFI_PSWD"\"\r\n", WIFISTAGE_CONNECTINGWLAN},
                              {"AT+CIPSTART=\"TCP\",\""WIFI_SRVIP"\","WIFI_SRVPORT"\r\n", WIFISTAGE_CONNECTINGSERVER},
                              {"", WIFISTAGE_READY},
                              {"", WIFISTAGE_READY},
                              {"", WIFISTAGE_SENDING},
                              {"", WIFISTAGE_READY}};

WIFISTAGE_TYPE Fail_Step[] = {WIFISTAGE_RESETTING,
                              WIFISTAGE_RESETTING,
                              WIFISTAGE_RESETTING,
                              WIFISTAGE_READY,
                              WIFISTAGE_READY,
                              WIFISTAGE_SENDING,
                              WIFISTAGE_READY};

#endif
