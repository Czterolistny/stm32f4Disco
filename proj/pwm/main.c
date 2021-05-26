#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_adc.h"
#include "../usart/uart.h"
#include "../common/dbg_pin.h"
#include <stdlib.h>

volatile uint32_t msTicks = 0;

void delay_ms(uint32_t ms)
{
	uint32_t ticks = msTicks + ms;
	
	while(ticks > msTicks);
	while(ticks < msTicks);

}

void SysTick_Handler()
{
	msTicks++;
}

void TIM7_IRQHandler()
{

    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		ToggleTestPin1();
		ADC_SoftwareStartConv(ADC1);
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}

#define PWM_TIM_PERIOD (2000 - 1)
#define PWM_TIM_PRESC (840 - 1)

TIM_OCInitTypeDef TIM_OCStruct;
TIM_TimeBaseInitTypeDef TIM_BaseStruct;

volatile int adc_channel = 0;
void ADC_IRQHandler()
{
	if( ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET )
	{
		//float pwm_correction;
		uint16_t adc = ADC_GetConversionValue(ADC1);
		if( adc > 4000 )
			adc = 4000;
		
		switch( adc_channel ){
			case 1:
				//TIM4->ARR =  PWM_TIM_PERIOD + (adc << 2); 
				TIM4->PSC = PWM_TIM_PRESC + adc;
				adc_channel = 0;
				break;
			case 0:
				//pwm_correction = (float) TIM4->CCR1 / TIM4->ARR;
				TIM4->CCR2 = adc / 2;
				adc_channel++;
				break;
			default:
				break;
		}
		
		ToggleTestPin2();
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
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
	ADC_InitStruct.ADC_ExternalTrigConv     = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_DataAlign            = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion      = 2;
	ADC_Init(ADC1, &ADC_InitStruct);

	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_480Cycles);
	
	ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_SoftwareStartConv(ADC1);
}

void TIM7_Init()
{
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	//10ms
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 419;
    TIM_TimeBaseInitStruct.TIM_Period = 4999;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM7, ENABLE);
}


void TIM4_PWM_Init(void)
{
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_BaseStruct.TIM_Period = PWM_TIM_PERIOD;
  	TIM_BaseStruct.TIM_Prescaler = PWM_TIM_PRESC;
  	TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_BaseStruct.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM4, &TIM_BaseStruct);
	
	//OC PWM
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	
    TIM_OCStruct.TIM_Pulse = 0;//((PWM_TIM_PERIOD + 1) / 2) - 1; /* 50% duty cycle */
    TIM_OC2Init(TIM4, &TIM_OCStruct);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	//Pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	TIM_Cmd(TIM4, ENABLE);
}

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	ADC1_Init();
	TIM4_PWM_Init();
	TIM7_Init();
	
for (;;){};
}
