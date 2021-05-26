#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "../common/dbg_pin.h"
#include "uart.h"

uint8_t DMA_TEST_BUF[64];

void TxCompleteCallback(uint8_t tx_bytes)
{
	ToggleTestPin4();
}

void RxCompleteCallback(uint8_t *recv_buf, uint8_t recv_len)
{
	ToggleTestPin3();
	USART2_SendNoneBlocking(recv_buf, recv_len);
}

void initBuf(void)
{
	for(uint8_t i = 0; i < 64; ++i)
		DMA_TEST_BUF[i] = i + 1;
}

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

int main(void)
{  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	USART2_Init();
	
	char testStr[] = "Uart test\r\n";
	//USART2_SendString(&testStr[0]);
	initBuf();
	USART2_DMA_Send(&DMA_TEST_BUF[0], 64);
	
	//registerRxCompleteCallb(&RxCompleteCallback);
	//registerTxCompleteCallb(&TxCompleteCallback);
	//USART2_SendString(&testStr[0]);
	//
for (;;){};
}
