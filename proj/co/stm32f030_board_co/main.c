#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_misc.h"
#include "sregs.h"
#include "debugPins.h"

void delay_ms(uint32_t ms);
static void uartSend(uint8_t *tx_buf, uint8_t len);

void testUart1(void)
{
	const uint8_t sendBuf[] = "test to send\n";
	for(;;)
	{
		clearTestPin();
		uartSend((uint8_t*) &sendBuf[0], sizeof(sendBuf)/sizeof(sendBuf[0]));
		delay_ms(150);
		setTestPin();
		delay_ms(150);
	}
}

static void uartSend(uint8_t *tx_buf, uint8_t len)
{
	for(uint8_t i = 0; i < len; ++i)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, tx_buf[i]);
	}
}

void InitUsart1(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_0);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	USART_InitTypeDef USART_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = DISABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;

    NVIC_Init(&NVIC_InitStruct);

	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
}

volatile uint32_t msTicks;

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

int main()
{
	SystemInit();
    SysTick_Config(SystemCoreClock / 1000);
	initTestPin();
	
	InitUsart1();
	//sregsInit();
	
	testUart1();
	for(;;);
}
