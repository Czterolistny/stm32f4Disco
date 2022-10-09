#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_i2c.h"
#include "uart.h"
#include "dbg_pin.h"
#include <stdbool.h>

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

void I2C1_ER_IRQHandler()
{
	if( I2C_GetITStatus(I2C1, I2C_IT_AF | I2C_IT_BERR | I2C_IT_ARLO | I2C_IT_TIMEOUT) != RESET )
	{
		if( I2C_GetFlagStatus(I2C1, I2C_IT_AF) ){
			SetTestPin2();
		}else if( I2C_GetFlagStatus(I2C1, I2C_IT_BERR) ){
			SetTestPin3();
		}else if( I2C_GetFlagStatus(I2C1, I2C_IT_ARLO) ){
			SetTestPin4();
		}else if( I2C_GetFlagStatus(I2C1, I2C_IT_TIMEOUT) ){
			SetTestPin4();
		}
		
		I2C_GenerateSTOP(I2C1, ENABLE);
		I2C_ClearITPendingBit(I2C1, I2C_IT_AF | I2C_IT_BERR | I2C_IT_ARLO | I2C_IT_TIMEOUT);
	}
}

void I2C1_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;// 7-SDA, 8-CLK
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	I2C_InitTypeDef I2C_InitStruct;	

	I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_Init(I2C1, &I2C_InitStruct);
	
	NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	//I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

}

void I2C1_write(uint8_t i2c_addr, uint8_t *write_buf, uint8_t reg, uint8_t size, bool setStop)
{
	I2C_GenerateSTART(I2C1, ENABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
 
	I2C_Send7bitAddress(I2C1, i2c_addr, I2C_Direction_Transmitter); 
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);

	for(int i = 0; i<size; i++){
		
		I2C_SendData(I2C1, write_buf[i]);
		while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS);
	}
	
	if( setStop == true ){
		I2C_GenerateSTOP(I2C1, ENABLE);
	}
}

void I2C1_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *read_buf, uint8_t size)
{
	I2C1_write(i2c_addr, &reg_addr, reg_addr, 1, false);
	
	I2C_GenerateSTART(I2C1, ENABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	I2C_Send7bitAddress(I2C1, i2c_addr, I2C_Direction_Receiver);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);
	
	for(int i = 0; i < size; ++i ) {
		
		while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
		
		if( i == size - 1 ){
			I2C_NACKPositionConfig(I2C1, I2C_NACKPosition_Current);
		}
		read_buf[i++] = I2C_ReceiveData(I2C1);
    }

	I2C_GenerateSTOP(I2C1, ENABLE);	
}

uint8_t msg[] = "i2c..\n";

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	//USART2_Init();
	I2C1_Init();

	//USART2_SendNoneBlocking(&msg[0], 6);
	I2C1_read(0x78, 0x01, &msg[0], 5);
	//USART2_SendNoneBlocking(&msg[0], 5);
	
	SetTestPin();
	
for (;;){};
}
