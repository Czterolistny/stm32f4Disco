#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "debugPins.h"

void initTestPin(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);	
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = TEST_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void setTestPin(void)
{
	GPIO_WriteBit(GPIOC, TEST_PIN, Bit_SET);
}

void clearTestPin(void)
{
    GPIO_WriteBit(GPIOC, TEST_PIN, Bit_RESET);
}

void toggleTestPin(void)
{
    if( Bit_SET == GPIO_ReadInputDataBit(GPIOC, TEST_PIN) )
    {
        GPIO_WriteBit(GPIOC, TEST_PIN, Bit_RESET); 
    }else
    {
        GPIO_WriteBit(GPIOC, TEST_PIN, Bit_SET);
    }
}