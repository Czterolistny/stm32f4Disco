#include <assert.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "dbg_pin.h"

void SetTestLed(uint16_t LEDx)
{
	assert( (LEDx <= LED1) && (LEDx >= LED4) );
	GPIO_SetBits(TEST_PORT, LEDx);
}
void ClearTestLed(uint16_t LEDx)
{
	assert( (LEDx <= LED1) && (LEDx >= LED4) );
	GPIO_ResetBits(TEST_PORT, LEDx);
}
void ToggleTestLed(uint16_t LEDx)
{
	assert( (LEDx <= LED1) && (LEDx >= LED4) );
	GPIO_ToggleBits(TEST_PORT, LEDx);
}


void InitTestPin(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = LED1 | LED2 | LED3 | LED4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Low_Speed;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_Init(GPIOD, &GPIO_InitStruct);
}

