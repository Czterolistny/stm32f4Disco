#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_spi.h"
#include "../usart/uart.h"
#include "../common/dbg_pin.h"

#define CS_PIN GPIO_Pin_3
#define CS_PORT GPIOE

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
	for(int i = 0; i<nmb; ++i)
	{
		SPI_SendData(SPI1, bytes[i]);
	}
}

void SPI1_Read(uint8_t *bytes, uint8_t nmb)
{
	for(int i = 0; i<nmb; ++i)
	{
		bytes[i] = SPI_ReceiveData(SPI1);
	}
}

void SPI1_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	 //PA5 SCK 6 MISO 7 MOSI
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_SetBits(GPIOE, GPIO_Pin_3);

	
	SPI_InitTypeDef SPI_InitStruct;
	SPI_StructInit(&SPI_InitStruct); 
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master; 
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI1, &SPI_InitStruct); 
	
	SPI_Cmd(SPI1, ENABLE);
	
}


uint8_t MX25x_Read_ManID(void)
{
	uint8_t id_cmd[] = {0x90, 0x00, 0x00, 0x00};
	CS_SET();
	SPI1_Write(&id_cmd[0], 4);
	uint8_t ManID;
	SPI1_Read(&ManID, 1);
	CS_RESET();
	
	return ManID;
}

uint8_t MX25x_Read_DevID(void)
{
	uint8_t id_cmd[] = {0x90, 0x00, 0x00, 0x01};
	CS_SET();
	SPI1_Write(&id_cmd[0], 4);
	uint8_t DevID;
	SPI1_Read(&DevID, 1);
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
	
	uint8_t id[3];
	id[0] = MX25x_Read_ManID();
	id[1] = MX25x_Read_DevID();
	id[2] = MX25x_Read_ElecID();
	USART2_SendNoneBlocking(&id[0], 3);
	
for (;;){};
}
