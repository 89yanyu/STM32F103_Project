#include "stm32f10x_exti.h"
void Init()
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE,
                           ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOE, GPIO_Pin_5);

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2);
    EXTI_InitStruct.EXTI_Line = EXTI_Line2;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
    EXTI_InitStruct.EXTI_Line = EXTI_Line3;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);


    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}

void Sleep(u32 ms)
{
    SysTick_Type *Tick = SysTick;
    u32 temp;
    ms *= 8000;

    Tick->LOAD = ms;
    Tick->VAL = 0;
    Tick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    do {
        temp = Tick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));

    Tick->VAL = 0;
    Tick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void ChangeBit(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
    if (GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin) == 1)
    {
        GPIO_ResetBits(GPIOx, GPIO_Pin);
    }
    else
    {
        GPIO_SetBits(GPIOx, GPIO_Pin);
    }
}

/* KEY2->B5 */
void EXTI0_IRQHandler(void)
{
    //if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        Sleep(10);
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            GPIO_ResetBits(GPIOE, GPIO_Pin_5);
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    return;
}
/* KEY2->B5 */
void EXTI2_IRQHandler(void)
{
    //if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        Sleep(10);
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0)
        {
            ChangeBit(GPIOB, GPIO_Pin_5);
        }
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
    return;
}
/* KEY1->B5,E5 */
void EXTI3_IRQHandler(void)
{
    //if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        Sleep(10);
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
        {
            ChangeBit(GPIOB, GPIO_Pin_5);
            ChangeBit(GPIOE, GPIO_Pin_5);
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
    return;
}
/* KEY0->E5 */
void EXTI4_IRQHandler(void)
{
    //if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        Sleep(10);
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
        {
            ChangeBit(GPIOE, GPIO_Pin_5);
        }
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
    return;
}

int main()
{
    Init();
    ChangeBit(GPIOB, GPIO_Pin_5);
    ChangeBit(GPIOE, GPIO_Pin_5);
    Sleep(500);
    ChangeBit(GPIOB, GPIO_Pin_5);
    ChangeBit(GPIOE, GPIO_Pin_5);
    Sleep(500);
    ChangeBit(GPIOB, GPIO_Pin_5);
    ChangeBit(GPIOE, GPIO_Pin_5);
    Sleep(500);
    ChangeBit(GPIOB, GPIO_Pin_5);
    ChangeBit(GPIOE, GPIO_Pin_5);
    Sleep(500);
    ChangeBit(GPIOB, GPIO_Pin_5);
    ChangeBit(GPIOE, GPIO_Pin_5);
    Sleep(500);
    ChangeBit(GPIOB, GPIO_Pin_5);
    ChangeBit(GPIOE, GPIO_Pin_5);
    Sleep(500);
    for(;;);
}
