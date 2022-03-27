#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_tim.h"
#include "sregs.h"
#include "debugPins.h"
//#include "esp.h"

#define FAN_PERC_ADDR 	((uint8_t) 0x3F)
#define SET_TEMP_ADDR 	((uint8_t) 0x23)
#define TEMP1_ADDR 		((uint8_t) 0x1E)
#define TEMP2_ADDR 		((uint8_t) 0x26)
#define EXH_TEMP_ADDR 	((uint8_t) 0x42)

#define NULL ((void*)0) 

const uint8_t param[] = {FAN_PERC_ADDR, SET_TEMP_ADDR, TEMP1_ADDR, TEMP2_ADDR, EXH_TEMP_ADDR};

const uint8_t uart_respond[] = {0x02, 0x26, 0xff, 0xf4, 0x16, 0xf9, 0x00, 0x01, 0x16, 0xc2, 0x00, 0x00, 0x16, 0xf9, 0x00, 0x00,
								0x16, 0xf9, 0x00, 0x02, 0x16, 0xc2, 0x00, 0x00, 0x16, 0xf9, 0x00, 0x00, 0x02, 0x18, 0x2c, 0x11};
uint8_t *pTX_BUF = NULL;
uint8_t rx_buf[128];

volatile int8_t delay_5ms_count = -1;
volatile uint8_t frame_byte_rec;
volatile uint8_t tx_cnt;

volatile uint32_t msTicks;

void delay_ms(uint32_t ms);
static void uartSend(uint8_t *tx_buf, uint8_t len);

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
		USART_ClearFlag(USART1, USART_IT_RXNE);
		rx_buf[frame_byte_rec++] = (uint8_t) USART_ReceiveData(USART1);
		delay_5ms_count = 0;
		
    }else if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
		static uint8_t txed_cnt;
		if(txed_cnt != tx_cnt)
		{
			USART_SendData(USART1, pTX_BUF[++txed_cnt]);
		}else {
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
			txed_cnt = 0;
			tx_cnt = 0;
		}
    }
}

void TIM3_IRQHandler()
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
		if( delay_5ms_count >= 0 ){
			delay_5ms_count++;
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

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

static void uartSendToCO(uint8_t *tx_buf, uint8_t len)
{
	pTX_BUF = tx_buf;
	USART_SendData(USART1, tx_buf[0]);
	tx_cnt = len - 1;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

static void uartSend(uint8_t *tx_buf, uint8_t len)
{
	for(uint8_t i = 0; i < len; ++i)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, tx_buf[i]);
	}
}

void processData(uint8_t *recv_buf, uint8_t recv_len)
{
	if( recv_len > 0x20 )
	{
		//sendToESP(recv_buf, recv_len);
	}	
	uartSendToCO((uint8_t *) &uart_respond[0], sizeof(uart_respond));
}

void TIM3_Init()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	//10ms
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 999;
    TIM_TimeBaseInitStruct.TIM_Period = 419;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
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
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;

    NVIC_Init(&NVIC_InitStruct);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
}

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
	TIM3_Init();
	//sregsInit();

	for (;;){
		while( delay_5ms_count < 10 );
		delay_5ms_count= -1;
		processData(&rx_buf[0], frame_byte_rec);
		frame_byte_rec = 0;
	};
}
