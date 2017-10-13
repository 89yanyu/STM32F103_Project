#include "timer.h"
#include "usart.h"
#include "encoder.h"

void GetSendSpeed()
{
    int speed = Encoder_GetSpeed();
    UIO_printf("%d\r\n", speed);
}

int main()
{
    UIO_Init(115200);
    UIO_SetAsync(ENABLE);
    Encoder_Init();
    Timer_Init();
    Timer_SetHandler(GetSendSpeed);
    for(;;)
    {
        UIO_AsyncSend();
    }
}
