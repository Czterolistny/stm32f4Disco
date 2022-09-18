#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_spi.h"
#include "uart.h"
#include "dbg_pin.h"

#define CS_PIN GPIO_Pin_4
#define CS_PORT GPIOA

#define CS_SET() GPIO_ResetBits(CS_PORT, CS_PIN);
#define CS_RESET() GPIO_SetBits(CS_PORT, CS_PIN);

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

void SPI1_Write(uint8_t *bytes, uint8_t nmb)
{
	for(uint8_t i = 0; i<nmb; ++i)
	{
		SPI_SendData(SPI1, bytes[i]);
		while(SPI_GetFlagStatus(SPI1, SPI_IT_TXE) == RESET);
		//while(SPI_GetFlagStatus(SPI1, SPI_IT_RXNE) == RESET);
		(uint8_t volatile) *((__IO uint8_t *)&(SPI1->DR));
	}
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);
}

void SPI1_Read(uint8_t *bytes, uint8_t nmb)
{
	for(uint8_t i = 0; i<nmb; ++i)
	{
		SPI_SendData(SPI1, 0u);
		while(SPI_GetFlagStatus(SPI1, SPI_IT_TXE) == RESET);
		//while(SPI_GetFlagStatus(SPI1, SPI_IT_RXNE) == RESET);
		*(bytes++) = SPI_ReceiveData(SPI1);
		ClearTestPin();
	}
}

void SPI1_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* SCK - PA5, MISO - PA6, MOSI - PA7 */
	/* Note - If stm32f4discovery keep PE3 high */
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	SPI_InitTypeDef SPI_InitStruct;
	SPI_StructInit(&SPI_InitStruct); 
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master; 
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI1, &SPI_InitStruct); 
	
	SPI_Cmd(SPI1, ENABLE);
	
}


void MX25x_Read_Ident(uint8_t *IdentBuf)
{
	uint8_t id_cmd = 0x9Fu;
	CS_SET();
	SPI1_Write(&id_cmd, 1u);
	SPI1_Read(IdentBuf, 3u);
	CS_RESET();
}

uint8_t MX25x_Read_DevID(void)
{
	uint8_t id_cmd[] = {0x90, 0x00, 0x00, 0x01};
	CS_SET();
	SPI1_Write(&id_cmd[0], 4);
	uint8_t DevID;
	SPI1_Read(&DevID, 3);
	CS_RESET();
	
	return DevID;
}

uint8_t MX25x_Read_ElecID(void)
{
	uint8_t id_cmd[] = {0xab, 0x00, 0x00, 0x00};
	CS_SET();
	SPI1_Write(&id_cmd[0], 4);
	uint8_t ElecID;
	SPI1_Read(&ElecID, 1);
	CS_RESET();
	return ElecID;
}

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	USART2_Init();
	SPI1_Init();
	
	SetTestPin();

	uint8_t id[3u];
	MX25x_Read_Ident(&id[0u]);
	USART2_SendBlocking(&id[0u], 3u);
	id[0] = MX25x_Read_DevID();
	id[1] = MX25x_Read_ElecID();
	USART2_SendBlocking(&id[0u], 2u);
	
for (;;){};
}
