#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"

void InitUsart1(void)
{
}

int main()
{
	SystemInit();
    SysTick_Config(SystemCoreClock / 1000);
	
	for(;;);
}
