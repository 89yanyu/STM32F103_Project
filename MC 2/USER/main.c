#include "stm32f10x_gpio.h"
#include "delay.h"
#include "usart.h"

const unsigned char show[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};
const uint16_t GPIO_Pin_LowByte = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
const uint16_t GPIO_Pin_HighByte = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
                                   GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
u32 time = 0;
void ShowNum()
{
    int bit, tmp;
    u16 load;
    bit = 0;
    tmp = time;
    while ((bit <= 7) || (tmp > 0))
    {
        load = 0xffff ^ ((1 << (7 - bit)) | (show[tmp % 10] << 8));
        if (bit == 0)
        {
            load ^= GPIO_Pin_15;
        }
        GPIOC->ODR&=0X0000;
        GPIOC->ODR|=load;
        tmp /= 10;
        bit++;
        Delay_ms_Lazy(2);
        GPIOC->ODR &= 0x0000;
    }
}

void LED8_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_LowByte;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_HighByte;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_All);
}

void RCC_Init()
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    if(RCC_WaitForHSEStartUp() == SUCCESS)
    {
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        FLASH_SetLatency(FLASH_Latency_2);
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while(RCC_GetSYSCLKSource() != 0x08);
    }
}

int main()
{
  RCC_Init();
  LED8_Init();
  Delay_Init();
  USART_MyInit();
  
  USART_SendFormat(USART_CONTROL_DEBUG, 8, (void*)"GOGOGO!!");
  long cnt = 0;
  time = 12345678;
  for (;;)
  {
      ShowNum();
      cnt++;
      if (cnt >= 100)
      {
          cnt = 0;
          //time++;
      }
  }
}
