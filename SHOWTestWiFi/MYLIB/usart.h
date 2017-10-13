#ifndef __89YY_USART__

#define __89YY_USART__

#include "stm32f10x.h"

void UIO_Init(u32);

u32 UIO_gets(char *buff);

u32 UIO_read(char *buff, int len);

u32 UIO_scanf(const char *format, ...);

u32 UIO_send(char *buff, int len);

u32 UIO_printf(const char *format, ...);

u8 UIO_hasNext(void);

void UIO_SetAsync(u8);

void UIO_AsyncSend(void);

#endif

