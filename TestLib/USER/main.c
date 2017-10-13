#include "beep.h"
#include "usart.h"
#include "delay.h"
#include "exinterrupt.h"
#include "led.h"
#include "string.h"
#include "key.h"

void WKUP_Rising()
{
    Delay_ms(10);
    if (GPIO_ReadInputDataBit(GPIO_WK_UP) == 0) return;
    UIO_printf("CHANGE ALL LED\r\n");
    LED_Change(LED_ALL);
}

void KEY2_Falling()
{
    Delay_ms(10);
    if (GPIO_ReadInputDataBit(GPIO_KEY_2) == 1) return;
    UIO_printf("CHANGE LED0\r\n");
    LED_Change(LED_0);
}

void KEY0_Falling()
{
    Delay_ms(10);
    if (GPIO_ReadInputDataBit(GPIO_KEY_0) == 1) return;
    UIO_printf("CHANGE LED1\r\n");
    LED_Change(LED_1);
}

void KEY1_Falling()
{
    Delay_ms(10);
    if (GPIO_ReadInputDataBit(GPIO_KEY_1) == 1) return;
    UIO_printf("TURNON ALL LED\r\n");
    LED_ON(LED_ALL);
}

int main()
{
    Delay_Init();
    LED_Init();
    Key_Init();
    Beep_Init();
    UIO_Init(115200);
    UIO_SetAsync(ENABLE);
    ExInterrupt_Init(GPIO_PortSourceGPIOA, GPIO_PinSource0, EXTI_Trigger_Rising, 0x02, 0x00);
    ExInterrupt_Init(GPIO_PortSourceGPIOE, GPIO_PinSource2, EXTI_Trigger_Falling, 0x02, 0x01);
    ExInterrupt_Init(GPIO_PortSourceGPIOE, GPIO_PinSource3, EXTI_Trigger_Falling, 0x02, 0x02);
    ExInterrupt_Init(GPIO_PortSourceGPIOE, GPIO_PinSource4, EXTI_Trigger_Falling, 0x02, 0x03);

    ExInterrupt_SetHandler(GPIO_Source_WK_UP, WKUP_Rising);
    ExInterrupt_SetHandler(GPIO_Source_KEY_0, KEY0_Falling);
    ExInterrupt_SetHandler(GPIO_Source_KEY_1, KEY1_Falling);
    ExInterrupt_SetHandler(GPIO_Source_KEY_2, KEY2_Falling);


    for(;;)
    {
        UIO_AsyncSend();
    }
}
