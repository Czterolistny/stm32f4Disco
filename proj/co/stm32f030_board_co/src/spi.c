#include <stdbool.h>
#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_misc.h"
#include "spi.h"
#include "sregs.h"

#define SPI (SPI1)

void inline spiSetCS_High(void)
{
    sregsSetOutput(SREGS_FLASH_CS, false);
}
void inline spiSetCS_Low(void)
{
    sregsSetOutput(SREGS_FLASH_CS, true);
}

void spiInit(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);

    SPI_InitTypeDef SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI, &SPI_InitStruct);

    spiSetCS_High();
    SPI_Cmd(SPI, ENABLE);

    /* By dafult RxFIFO is set to Full?? */
    SPI_RxFIFOThresholdConfig(SPI, SPI_RxFIFOThreshold_QF);
}

void spiWrite(uint8_t *buf, uint8_t len)
{
	for(uint8_t i = 0u; i < len; ++i)
	{
        SPI_SendData8(SPI, buf[i]);
		while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET){};
        while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET){};
		(uint8_t volatile) *((__IO uint8_t *)&(SPI->DR));
	}
	while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_BSY) == SET);
}

void spiWriteByte(uint8_t byte)
{
    spiWrite(&byte, 1u);
}

void spiRead(uint8_t *buf, uint8_t len)
{
    for(uint8_t i = 0u; i < len; ++i)
	{
        SPI_SendData8(SPI, 0u);
		while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET){};
        while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET){};
        buf[i] = SPI_ReceiveData8(SPI);
	}
}