#include "24cxx.h"
#include "usart.h"
#include "delay.h"

int main()
{
    AT24CXX_Init();
    UIO_Init(115200);
    while (AT24CXX_Check())
    {
        UIO_printf("AT24CXX Error!!\r\n");
        Delay_ms(10);
    }
    for (;;)
    {
        AT24CXX_WriteOneByte(0, 0x0ff);
    }
}