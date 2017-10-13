#include "stm32f10x_gpio.h"
#include "stdlib.h"

u8 fac_us;
u16 fac_ms;

void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	fac_us=SystemCoreClock/8000000;
	fac_ms=(u16)fac_us*1000;
}

void delay_ms(u16 nms)
{
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//????(SysTick->LOAD?24bit)
	SysTick->VAL =0x00;							//?????
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//????  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//??????   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//?????
	SysTick->VAL =0X00;       					//?????	
}

int main()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | 
	                       RCC_APB2Periph_GPIOE, ENABLE);
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_5);
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIO_SetBits(GPIOE,GPIO_Pin_5);

	delay_init();
  while(1)
	{
		int op = rand() % 4;
		if (op == 0) GPIO_SetBits(GPIOE,GPIO_Pin_5);
		else if (op == 1) GPIO_SetBits(GPIOB, GPIO_Pin_5);
		else if (op == 2) GPIO_ResetBits(GPIOE, GPIO_Pin_5);
		else if (op == 3) GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		delay_ms(100);
	}
}
