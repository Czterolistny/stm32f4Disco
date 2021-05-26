#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dcmi.h"
#include "../usart/uart.h"
#include "../common/dbg_pin.h"
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

void DMA2_Stream1_IRQHandler()
{
	if( DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1) != RESET )
	{
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
		ToggleTestPin3();
	}
}

volatile bool frame_received = false;
void DCMI_IRQHandler()
{
	if( DCMI_GetITStatus(DCMI_IT_FRAME) != RESET )
	{
		frame_received = true;
		DCMI_ClearITPendingBit(DCMI_IT_FRAME);
		ToggleTestPin2();
	}
	if( DCMI_GetITStatus(DCMI_IT_LINE) != RESET )
	{
		ToggleTestPin2();
		DCMI_ClearITPendingBit(DCMI_IT_LINE);
	}
	ToggleTestPin2();
}

#define FRAME_BUF_SIZE 50688
uint8_t FRAME_BUF[FRAME_BUF_SIZE];
volatile uint8_t *frame_ptr = (uint8_t*) &FRAME_BUF[0];
volatile bool tx_complete = false;

void TxCompleteCallback(uint8_t tx_bytes)
{
	frame_ptr += tx_bytes;
	tx_complete = true;
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
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_8;// 7-SDA, 8-CLK
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);
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

void I2C1_write(uint8_t i2c_addr, uint8_t *write_buf, uint8_t reg, uint8_t size)
{
	I2C_GenerateSTART(I2C1, ENABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
 
	I2C_Send7bitAddress(I2C1, i2c_addr, I2C_Direction_Transmitter); 
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);

	I2C_SendData(I2C1, reg);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS);
	
	for(int i = 0; i<size; i++){
		
		I2C_SendData(I2C1, write_buf[i]);
		while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS);
	}
	
	I2C_GenerateSTOP(I2C1, ENABLE);
}

void I2C1_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *read_buf, uint8_t size)
{
	I2C_GenerateSTART(I2C1, ENABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	I2C_Send7bitAddress(I2C1, i2c_addr, I2C_Direction_Transmitter); 
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
	
	I2C_SendData(I2C1, reg_addr);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS);
	
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

void initResetPin()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void initMCLK()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_MCO);
	
	RCC_PLLI2SCmd(DISABLE);
	RCC_PLLI2SConfig(192, 2);
	RCC_MCO1Config(RCC_MCO2Source_PLLI2SCLK, RCC_MCO2Div_5); //19.2MHz
	RCC_PLLI2SCmd(ENABLE);
}

void initDCMI()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_4, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_6, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_9, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_10, GPIO_AF_DCMI);
	
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_0, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_1, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_4, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_5, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_6, GPIO_AF_DCMI);
	
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_6, GPIO_AF_DCMI);
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_7, GPIO_AF_DCMI);
	
	GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz; // hsync, pixclk, 0, 1
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
								//2, 3, 4, 6, 7
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
								// 5, vsync
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	///////////////////////////////////////////////////////////////////////////
	DCMI_InitTypeDef DCMI_InitStruct;
	
	DCMI_StructInit(&DCMI_InitStruct);https://github.com/t27/stm32f4-dcmi-jpeg/blob/master/Project/OV9655_Camera/src/main.c
	DCMI_InitStruct.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;http://www.zhopper.narod.ru/mobile/tcm8210md_full.pdf
	DCMI_InitStruct.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
	DCMI_InitStruct.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;
	DCMI_InitStruct.DCMI_VSPolarity = DCMI_VSPolarity_High;
	DCMI_InitStruct.DCMI_HSPolarity = DCMI_HSPolarity_High;
	DCMI_InitStruct.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
	DCMI_InitStruct.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
	DCMI_Init(&DCMI_InitStruct);
	
	
	///////////////////////////////////////////////////////////////////////////
	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = DCMI_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);
	
	DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);
	DCMI_ITConfig(DCMI_IT_VSYNC, DISABLE);
	DCMI_ITConfig(DCMI_IT_LINE, DISABLE);
	DCMI_ITConfig(DCMI_IT_OVF, DISABLE);
	
	////////////////////////////////////////////////////////////////////////////
	DMA_InitTypeDef DMA_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Channel = DMA_Channel_1;
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t) &FRAME_BUF[0];
	DMA_InitStruct.DMA_BufferSize = FRAME_BUF_SIZE;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) &DCMI->DR;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init(DMA2_Stream1, &DMA_InitStruct);
	

	NVIC_InitStruct.NVIC_IRQChannel = DMA2_Stream1_IRQn;
  	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  	NVIC_Init(&NVIC_InitStruct);
	
	DMA1->HIFCR = DMA_LIFCR_CTCIF1;
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC | DMA_IT_TE, ENABLE);  
	
  	DMA_Cmd(DMA2_Stream1, ENABLE);
	DCMI_Cmd(ENABLE);
	DCMI_CaptureCmd(DISABLE);
	
}

void reset()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);
	delay_ms(1);
	GPIO_SetBits(GPIOB, GPIO_Pin_4);
	delay_ms(1);
}

#define CAM_I2C_ADDR 0x78

uint8_t buf[8];
void initCam()
{
	initMCLK();
	
	initResetPin();
	reset();
	
	buf[0] = 0xD0; buf[1] = 0x1A;
	I2C1_write(CAM_I2C_ADDR, &buf[0], 0x02, 2);
	buf[0] = 0x48;
	I2C1_write(CAM_I2C_ADDR, &buf[0], 0x1E, 1);
	
	delay_ms(1);
	I2C1_read(CAM_I2C_ADDR, 0x01, &buf[0], 8);
	//USART2_SendNoneBlocking(&buf[0], 8);
	
	initDCMI();
	DCMI_CaptureCmd(ENABLE);

}

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	USART2_Init();
	registerTxCompleteCallb((void*) &TxCompleteCallback);
	
	I2C1_Init();
	initCam();
		
	SetTestPin();
	
	for(;;){
		if( frame_received == true ){
			USART2_SendNoneBlocking( frame_ptr, ( frame_ptr + FRAME_BUF_SIZE ) - frame_ptr);
			while(tx_complete == false);
			tx_complete = false;
		}
	}
}
