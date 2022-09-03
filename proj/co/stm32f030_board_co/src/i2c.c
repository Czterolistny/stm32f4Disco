#include <stdbool.h>
#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_misc.h"
#include "i2c.h"


static void i2c1Init(void);
static void i2c1Write(uint8_t i2c_addr, uint8_t *write_buf, uint8_t reg_addr, uint8_t size);
static void i2c1Read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *read_buf, uint8_t size);

void i2cInit(void)
{
    i2c1Init();

    uint8_t buf[3] = {5u, 0u, 0u};

    i2c1Write(0xa2u, &buf[0], 0x09, 1u);
    delay_ms(4);
    i2c1Read(0xa2u, 0x09u, &buf[0], 1u);
}

void i2cWrite(uint8_t *buf, uint8_t len)
{
    i2c1Write(buf[0], buf, buf[1], len);
}

void i2cRead(uint8_t *buf, uint8_t len)
{
    i2c1Read(buf[0], buf[1], buf, len);
}

static void i2c1Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_1);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	I2C_InitTypeDef I2C_InitStruct;	

	I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Disable;
    I2C_InitStruct.I2C_DigitalFilter = 0x00;
	I2C_InitStruct.I2C_Timing = 0x0070D8FF;//0x00E0257A;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_Init(I2C1, &I2C_InitStruct);
	
	NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2C1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	//I2C_ITConfig(I2C1, I2C_IT_TXIS, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

}

static void i2c1Read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *read_buf, uint8_t size)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);

    I2C_TransferHandling(I2C1, i2c_addr, 1u, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

    I2C_SendData(I2C1, reg_addr);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TC) == RESET);

    I2C_TransferHandling(I2C1, i2c_addr, size, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    for(uint8_t i = 0u; i < size; ++i)
    {	
        while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
        read_buf[i] = I2C_ReceiveData(I2C1);
    }
    
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
    I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);
}

static void i2c1Write(uint8_t i2c_addr, uint8_t *write_buf, uint8_t reg_addr, uint8_t size)
{
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);

    I2C_TransferHandling(I2C1, i2c_addr, 1u, I2C_Reload_Mode, I2C_Generate_Start_Write);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

    I2C_SendData(I2C1, reg_addr);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TCR) == RESET);

	I2C_TransferHandling(I2C1, i2c_addr, size, I2C_AutoEnd_Mode, I2C_No_StartStop);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

	for(uint8_t i = 0u; i < size; ++i)
    {	
        I2C_SendData(I2C1, write_buf[i]);
        while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TC) == SET);
    }

    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
    I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);
}