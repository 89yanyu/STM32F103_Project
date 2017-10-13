#ifndef __89YY_I2C__

#define __89YY_I2C__

#include "stm32f10x.h"

void I2C_MyInit(void);

void I2C_Start(void);

void I2C_Stop(void);

void I2C_WriteByte(u8 txd);

void I2C_ACK(void);

void I2C_NACK(void);

u8 I2C_WaitforACK(void);

u8 I2C_ReadByte(void);

void I2C_WriteByte(u8 txd);

#endif
