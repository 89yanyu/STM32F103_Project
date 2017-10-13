#ifndef __89YY_USART__

#define __89YY_USART__

#include "stm32f10x.h"

typedef enum {
  USART_CONTROL_ACK = 0,
  USART_CONTROL_NAK,
  USART_CONTROL_START,
  USART_CONTROL_STOP,
  USART_CONTROL_SETSPEED,
  USART_CONTROL_SETPID,
  USART_CONTROL_STATE,
  USART_CONTROL_DEBUG,
  USART_CONTROL_MAX
} USART_CONTROL_TYP;

void USART_MyInit(void);

void USART_SendFormat(u8 control, u16 len, u8 * ptr);

u8 USART_GetData(USART_CONTROL_TYP *op, u16 *len, u8 **data);

#endif
