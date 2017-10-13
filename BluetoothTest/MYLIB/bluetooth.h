#ifndef __89YY_HBUSART__

#define __89YY_HBUSART__

#include "stm32f10x.h"

typedef enum {
  HBUSART_CONTROL_ACK = 0,
  HBUSART_CONTROL_NAK,
  HBUSART_CONTROL_FRONT,
  HBUSART_CONTROL_BACK,
  HBUSART_CONTROL_STOP,
  HBUSART_CONTROL_TURNLEFT,
  HBUSART_CONTROL_TURNRIGHT,
  HBUSART_CONTROL_ON,
  HBUSART_CONTROL_OFF,
  HBUSART_CONTROL_SETSPEED,
  HBUSART_CONTROL_SETPID,
  HBUSART_CONTROL_STATE,
  HBUSART_CONTROL_DEBUG,
  HBUSART_CONTROL_ALIVE,
  HBUSART_CONTROL_MAX
} HBUSART_CONTROL_TYP;

void HBUSART_MyInit(void);

void HBUSART_SendFormat(u8 control, u8 len, u8 *ptr);

u8 HBUSART_GetData(HBUSART_CONTROL_TYP *op, u8 *data);

#endif