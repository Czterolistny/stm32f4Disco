#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
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
	msTicks++;
}

void TIM7_IRQHandler()
{

    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
		
		EXTI_ClearITPendingBit(EXTI_Line0);
		ToggleTestPin4();
		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0 )
		{
			NVIC_EnableIRQ(EXTI0_IRQn);
			TIM_Cmd(TIM7, DISABLE);
			ClearTestPin4();
			//NVIC_DisableIRQ(TIM7_IRQn);//TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
		}
		
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}


void EXTI0_IRQHandler(void) 
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
	{
		NVIC_DisableIRQ(EXTI0_IRQn);
		ToggleTestPin();
		EXTI_ClearITPendingBit(EXTI_Line0);
		//NVIC_EnableIRQ(TIM7_IRQn);//TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM7, ENABLE);
    }
}

void TIM7_Init(void)
{
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	//100ms
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 1999;
    TIM_TimeBaseInitStruct.TIM_Period = 4199;
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
    TIM_Cmd(TIM7, DISABLE);
}


void EXTIRQ_Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();
	
	EXTIRQ_Init();
	TIM7_Init();

	SetTestPin();
	
for (;;){};
}
