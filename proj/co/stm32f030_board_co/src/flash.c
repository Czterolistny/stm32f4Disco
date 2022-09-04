#include "flash.h"
#include "stm32f0xx.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "sregs.h"

#define FLASH_READ_STATUS_REG       (0x05u)
#define FLASH_READ_UNIQUE_ID        (0x4Bu)
#define FLASH_READ_MANUF_DEVICE_ID  (0x90u)
#define FLASH_READ_IDENTIFIC        (0x9Fu)

uint8_t flashBuf[3u];
void flashTest(void)
{
    sregsSetOutput(SREGS_FLASH_CS, false);

    SPI_SendData8(SPI1, FLASH_READ_IDENTIFIC);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_IT_TXE) == SET);

    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_IT_RXNE) == RESET);
    flashBuf[0u] = SPI_ReceiveData8(SPI1);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_IT_RXNE) == RESET);
    flashBuf[1u] = SPI_ReceiveData8(SPI1);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_IT_RXNE) == RESET);
    flashBuf[2u] = SPI_ReceiveData8(SPI1);

    sregsSetOutput(SREGS_FLASH_CS, true);
}

void flashInit(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // PA5 SCK 6 MISO 7 MOSI
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
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
    SPI_Init(SPI1, &SPI_InitStruct);

    SPI_Cmd(SPI1, ENABLE);

    flashTest();
}