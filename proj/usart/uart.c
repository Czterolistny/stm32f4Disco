#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include "dbg_pin.h"
#include "uart.h"

#define NULL ((void*) 0)

uint8_t TX_BUF[TX_BUF_SIZE];
uint8_t RX_BUF[RX_BUF_SIZE];
uint8_t DMA_BUF[DMA_BUF_SIZE];

static volatile uint8_t *ptr_DMA_BUF;
static uint8_t dma_len;
static uint8_t tx_cnt;
static volatile uint8_t rx_cnt;

typedef void (*RxCompleteCallb)(uint8_t *, uint16_t);
RxCompleteCallb rxComplete = NULL;
typedef void (*TxCompleteCallb) (uint16_t);
TxCompleteCallb txComplete = NULL;

uint8_t registerRxCompleteCallb(RxCompleteCallb callback)
{
	if(callback == NULL)
		return 1;
	else
		rxComplete = callback;
	
	return 0;
}

uint8_t registerTxCompleteCallb(TxCompleteCallb callback)
{
	if(callback == NULL)
		return 1;
	else
		txComplete = callback;
	
	return 0;
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
       	ToggleTestPin();
		USART_ClearFlag(USART2, USART_IT_RXNE);
		if(rx_cnt < RX_BUF_SIZE){
			RX_BUF[rx_cnt++] = (uint8_t) USART_ReceiveData(USART2);
		}
		USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    }else if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
		static uint8_t tx_irq_cnt;
		if(tx_irq_cnt == tx_cnt){
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
			if( txComplete != NULL ){
				txComplete(tx_cnt);
			}
			tx_irq_cnt = 0;
			tx_cnt = 0;
		}else{
			USART_SendData(USART2, TX_BUF[++tx_irq_cnt]);
		}
		//ToggleTestPin();
		
    }else if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART_ClearFlag(USART2, USART_IT_IDLE);
		USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
		if(rx_cnt > 0){
			USART_SendData(USART2, RX_BUF[0]);
			if( rxComplete != NULL ){

				rxComplete(&RX_BUF[0], rx_cnt);
			}
			rx_cnt = 0;
		}
	}
}

void DMA1_Stream5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5) != RESET)
    {
		ToggleTestPin2();
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
       //
        //len = DMA_BUFFER_SIZE - DMA1_Stream5->NDTR;
        //dataToCopy = UART_BUFFER_SIZE - WriteData;    
       //
        //if (dataToCopy > len) { dataToCopy = len; }
       //
        ///* Write received data to main buffer */
        //pointerToData = DMA_Buffer;
        //memcpy(&UART_Buffer[WriteData], pointerToData, dataToCopy);  
       //
        ///* Correct values for remaining data */
        //WriteData += dataToCopy;
        //len -= dataToCopy;
        //pointerToData += dataToCopy;
        //       
        ///* Copy rest data to buffer */
        //if (len) {
        //    memcpy(&UART_Buffer[0], pointerToData, len);
        //    WriteData = len;
        //}
        //       
        ///* Prepare DMA for next transfer, clear all flags */
        //DMA_ClearFlag(DMA1_Stream5, (DMA_FLAG_DMEIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5) );
        //DMA1_Stream5->M0AR = (uint32_t)DMA_Buffer;      /* Set memory address for DMA again */
        //DMA1_Stream5->NDTR = DMA_BUFFER_SIZE;               /* Set number of bytes to receive */
        //DMA_Cmd(DMA1_Stream5, ENABLE);
        //       
        //memset(DMA_Buffer, 0, sizeof(DMA_Buffer));
    }

}

void DMA1_Stream6_IRQHandler(void)
{
	if( DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET )
    {
		
		uint8_t copyDataLen;
		
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
        
		if (dma_len >= DMA_BUF_SIZE) 
			copyDataLen = DMA_BUF_SIZE; 
		else
			copyDataLen = dma_len;
		
		dma_len -= copyDataLen;
		
		for(int i = 0; i < copyDataLen; ++i)
			DMA_BUF[i] = *ptr_DMA_BUF++;

		DMA_ClearFlag(DMA1_Stream6, (DMA_FLAG_DMEIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_TCIF6 | DMA_FLAG_TEIF6) );		
        //DMA1_Stream6->M0AR = (uint32_t) &DMA_BUF[0];
        DMA1_Stream6->NDTR = copyDataLen;
        DMA_Cmd(DMA1_Stream6, ENABLE);
    }
	
	if( DMA_GetITStatus(DMA1_Stream6, DMA_IT_TEIF6 | DMA_IT_DMEIF6) != RESET )
	{
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TEIF6 | DMA_IT_DMEIF6);
		SetTestPin3();
	}

}

void USART2_Init(void)
{
	//GPIO
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	USART_InitTypeDef USART_InitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStruct);
	USART_Cmd(USART2, ENABLE);
	
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);

	//DMA
	DMA_InitTypeDef DMA_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_Channel = DMA_Channel_4;
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t) &DMA_BUF[0];
	DMA_InitStruct.DMA_BufferSize = DMA_BUF_SIZE;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) &USART2->DR;
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init(DMA1_Stream6, &DMA_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  	NVIC_Init(&NVIC_InitStruct);
	
  	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  	NVIC_Init(&NVIC_InitStruct);

	DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CTCIF5;
	
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_TE | DMA_IT_DME, ENABLE);
  	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC | DMA_IT_TE | DMA_IT_DME, ENABLE);  
  	DMA_Cmd(DMA1_Stream5, DISABLE);
  	DMA_Cmd(DMA1_Stream6, DISABLE);
   
}

uint8_t USART2_DMA_Send(uint8_t *bytes, uint16_t len)
{
	if (DMA1_Stream6->NDTR) {
        //return 0;
    }

	ptr_DMA_BUF = bytes;
	dma_len = len;
	
	if( len >= DMA_BUF_SIZE )
		len = DMA_BUF_SIZE;
	else
		DMA1_Stream6->NDTR = len;
	
	dma_len -= len; 
	
	for(int i = 0; i < len; ++i)
		DMA_BUF[i] = *ptr_DMA_BUF++;

	DMA_ClearFlag(DMA1_Stream6, (DMA_FLAG_DMEIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_TCIF6 | DMA_FLAG_TEIF6) );
   
    DMA_Cmd(DMA1_Stream6, ENABLE);
    USART2->CR3 |= USART_CR3_DMAT;

	return 1;
}

uint8_t USART2_SendNoneBlocking(uint8_t *bytes, uint16_t len)
{
	if( tx_cnt != 0 )
		return 1;
	
	if(len > TX_BUF_SIZE)
		len = TX_BUF_SIZE;
	
	tx_cnt = len;
	for(uint8_t i = 0; i < len; ++i)
		TX_BUF[i] = bytes[i];

	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, TX_BUF[0]); 
	tx_cnt--;
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	return 0;
}

void USART2_SendBlocking(uint8_t *bytes, uint16_t len)
{
	for(int i = 0; i < len; ++i){
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		USART_SendData(USART2, bytes[i]);
	}
}

void USART2_Send_Byte(uint8_t byte)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, byte);
}

void USART2_SendString(char *str)
{
	uint8_t size = 0; char *ptr = str;
	while(*str++)
		size++;
	USART2_SendNoneBlocking((uint8_t *) ptr, size);
}
