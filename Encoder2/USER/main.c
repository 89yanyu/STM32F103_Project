#include "exinterrupt.h"
#include "timer.h"
#include "led.h"
#include "usart.h"
#include "delay.h"
#include "key.h"

int Count = 0;

void UpdateTimer(void)
{
    u16 time = 0;
    if ((TIM3->CR1 & TIM_CR1_CEN) != 0)
    {
        TIM_Cmd(TIM3, DISABLE);
        time = TIM3->CNT;
        UIO_printf("Time %d\r\n", time);
    }
    else
    {
        UIO_printf("Time inf\r\n");
    }
    TIM3->CNT = 0;
    TIM_Cmd(TIM3, ENABLE);
}

void Handler0(void)
{
    u8 now = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2);
    Delay_ms_Lazy(10);
    if (now == GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)) return;
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) != GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
    {
        Count++;
        UIO_printf("++ 0 %d\r\n", Count);
    }
    else
    {
        Count--;
        UIO_printf("-- 0 %d\r\n", Count);
    }
    LED_Change(LED_0);
    if (Count == 4)
    {
        UIO_printf("Positive\r\n");
        UpdateTimer();
        Count = 0;
    }
    else if (Count == -4)
    {
        UIO_printf("Negtive\r\n");
        UpdateTimer();
        Count = 0;
    }
}

void Handler1(void)
{
    u8 now = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3);
    Delay_ms_Lazy(10);
    if (now == GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)) return;
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
    {
        Count++;
        UIO_printf("++ 1 %d\r\n", Count);
    }
    else
    {
        Count--;
        UIO_printf("-- 1 %d\r\n", Count);
    }
    LED_Change(LED_1);
    if (Count == 4)
    {
        UIO_printf("Positive\r\n");
        UpdateTimer();
        Count = 0;
    }
    else if (Count == -4)
    {
        UIO_printf("Negtive\r\n");
        UpdateTimer();
        Count = 0;
    }
}

void TimerHandler(void)
{
    UIO_printf("Timeout\r\n");
    TIM_Cmd(TIM3, DISABLE);
}

int main()
{
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    Delay_Init();
    LED_Init();
    UIO_Init(115200);
    UIO_SetAsync(ENABLE);
    Timer_Init();
    Timer_SetHandler(TimerHandler);
    Key_Init();
    ExInterrupt_Init(GPIO_PortSourceGPIOE, GPIO_PinSource2, EXTI_Trigger_Rising_Falling, 2, 2);
    ExInterrupt_Init(GPIO_PortSourceGPIOE, GPIO_PinSource3, EXTI_Trigger_Rising_Falling, 2, 2);
    ExInterrupt_SetHandler(GPIO_PortSourceGPIOE, GPIO_PinSource2, Handler0);
    ExInterrupt_SetHandler(GPIO_PortSourceGPIOE, GPIO_PinSource3, Handler1);

    for (;;)
    {
        UIO_AsyncSend();
    }
}
