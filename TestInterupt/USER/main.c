#include "exinterrupt.h"
#include "led.h"
#include "delay.h"
#include "key.h"

void WKUP_Rising()
{
    Delay_ms_Lazy(10);
    if (GPIO_ReadInputDataBit(GPIO_WK_UP) == 0) return;
    LED_Change(LED_0);
}

int main()
{
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    LED_Init();
    Delay_Init();
    Key_Init();
    ExInterrupt_Init(GPIO_PortSourceGPIOA, GPIO_PinSource0, EXTI_Trigger_Rising, 0x02, 0x00);
    ExInterrupt_SetHandler(GPIO_PortSourceGPIOA, GPIO_PinSource0, WKUP_Rising);
    while (1)
    {
        __set_PRIMASK(1);
        LED_ON(LED_1);
        Delay_ms_Lazy(5000);
        LED_OFF(LED_1);
        __set_PRIMASK(0);
        Delay_ms_Lazy(5000);
    }
}
