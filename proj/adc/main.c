#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "uart.h"
#include "dbg_pin.h"

volatile uint32_t msTicks = 0;

void delay_ms(uint32_t ms)
{
	uint32_t ticks = msTicks + ms;
	
	while(ticks > msTicks);
	while(ticks < msTicks);

}

void SysTick_Handler()
{
	if( (msTicks % 50) == 0 ){
		ADC_SoftwareStartConv(ADC1);
	}
	
	msTicks++;
}

void ADC_IRQHandler()
{
	if( ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET )
	{
		uint8_t val = ADC_GetConversionValue(ADC1);
		USART2_SendNoneBlocking(&val, 1);
		ToggleTestPin();
	}
	
}

void ADC1_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
			
	ADC_InitTypeDef	ADC_InitStruct;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_CommonInitStruct.ADC_Mode             = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler        = ADC_Prescaler_Div8;
	ADC_CommonInitStruct.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);

	ADC_InitStruct.ADC_Resolution           = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode         = ENABLE;
	ADC_InitStruct.ADC_ContinuousConvMode   = DISABLE; // ENABLE for max ADC sampling frequency
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	//ADC_InitStruct.ADC_ExternalTrigConv     = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_DataAlign            = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion      = 2;
	ADC_Init(ADC1, &ADC_InitStruct);

	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_480Cycles);

	ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_SoftwareStartConv(ADC1);
}

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	USART2_Init();
	ADC1_Init();
	
for (;;){};
}
