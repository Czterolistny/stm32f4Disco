#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "../common/dbg_pin.h"
#include "led_disp.h"

const uint8_t uart_respond[] = {0x02, 0x26, 0xff, 0xf4, 0x16, 0xf9, 0x00, 0x01, 0x16, 0xc2, 0x00, 0x00, 0x16, 0xf9, 0x00, 0x00,
								0x16, 0xf9, 0x00, 0x02, 0x16, 0xc2, 0x00, 0x00, 0x16, 0xf9, 0x00, 0x00, 0x02, 0x18, 0x2c, 0x11};
uint8_t *pTX_BUF = NULL;
uint8_t rx_buf[128];

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

#define FAN_PERC 	0
#define SET_TEMP 	1
#define TEMP1 		2
#define TEMP2		3
#define EXH_TEMP	4

#define FAN_PERC_ADDR 0x3F
#define SET_TEMP_ADDR 0x23
#define TEMP1_ADDR 0x1E
#define TEMP2_ADDR 0x26
#define EXH_TEMP_ADDR 0x42

const uint8_t param[] = {FAN_PERC_ADDR, SET_TEMP_ADDR, TEMP1_ADDR, TEMP2_ADDR, EXH_TEMP_ADDR};
volatile uint8_t param_iter = 0;
volatile int8_t delay_5ms_ticks = -1;
volatile uint8_t frame_byte_rec = 0;
volatile uint8_t tx_cnt;

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
		USART_ClearFlag(USART2, USART_IT_RXNE);
		rx_buf[frame_byte_rec++] = (uint8_t) USART_ReceiveData(USART2);
		delay_5ms_ticks = 0;
		
    }else if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
		static uint8_t txed_cnt;
		if(txed_cnt == tx_cnt){
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
			txed_cnt = 0;
			tx_cnt = 0;
		}else {
			USART_SendData(USART2, pTX_BUF[++txed_cnt]);
		}
    }
}

uint8_t *at_pTX_BUF = NULL;
uint8_t at_rx_buf[16];
volatile uint8_t at_rx_cnt;
volatile uint8_t at_txed_cnt;
volatile uint8_t at_tx_cnt;

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
		USART_ClearFlag(USART3, USART_IT_RXNE);
		at_rx_buf[at_rx_cnt++] = (uint8_t) USART_ReceiveData(USART3);
		
    }else if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
		static uint8_t at_txed_cnt;
		if(at_txed_cnt == at_tx_cnt){
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			at_txed_cnt = 0;
			at_tx_cnt = 0;
		}else {
			USART_SendData(USART3, at_pTX_BUF[++at_txed_cnt]);
		}
    }else if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET){
		
		USART_ClearFlag(USART2, USART_IT_IDLE);
		USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
		
		if(at_rx_cnt > 0){
		//......................///
			at_rx_cnt = 0;
		}
	}
}

void uartSend(uint8_t *tx_buf, uint8_t len)
{
	pTX_BUF = tx_buf;
	USART_SendData(USART2, tx_buf[0]);
	tx_cnt = len - 1;
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void sendATcmd(char *cmd, uint8_t len)
{
	//at_pTX_BUF = cmd;
	//USART_SendData(USART3, cmd[0]);
	//at_tx_cnt = len - 1;
	//USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	for(uint8_t i = 0; i < len; ++i){
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
		USART_SendData(USART3, cmd[i]);
	}
}

uint8_t uint_to_string(char *s, uint16_t x)
{
	uint8_t i = 0;
	if(x == 0){
		*s++ = '0';
		*s++ = ' ';
		i += 2;
		return i;
	}
	
	uint16_t div = 10000;
	while( div ){
		if( x/div ){
			*s++ = ((x/div) - (x / (div*10)) * 10) + '0';
			i++;
		}
		div = div / 10;
	}
	*s = ' ';
	i++;
	return i;
}

uint8_t _strlen(char *s)
{
	uint8_t len = 0;
	while( *s++ != 0 )
		len++;
	return len;
}

#define CRC_INIT 0xB169
uint16_t crc16calc(char *buf, uint16_t crc, uint16_t len)
{
	uint16_t i, tmp_crc = crc, poly=0x2f5c;

	while (len--)
    {
		tmp_crc = *buf++ << 8;
	    crc = tmp_crc ^ crc;

	    for(i = 0; i < 8; ++i)
        {
		    if(crc > 32767)
			 	crc = poly ^ (crc << 1);
            else
			    crc = crc << 1;
			
			crc = crc & 0xFFFF;
        }
    }

    return crc;
}

void sendToESP(uint8_t *recv_buf, uint8_t recv_len)
{
	char udp_buf[32];
	
	uint8_t buf_len = uint_to_string(&udp_buf[0], recv_buf[param[SET_TEMP]]);
	buf_len += uint_to_string(&udp_buf[buf_len], ((recv_buf[param[TEMP1]] << 8) | recv_buf[param[TEMP1] +1]) / 10);
	buf_len += uint_to_string(&udp_buf[buf_len], ((recv_buf[param[TEMP2]] << 8) | recv_buf[param[TEMP2] +1]) / 10);
	buf_len += uint_to_string(&udp_buf[buf_len], ((recv_buf[param[EXH_TEMP]] << 8) | recv_buf[param[EXH_TEMP] +1]) / 10);
	buf_len += uint_to_string(&udp_buf[buf_len], recv_buf[param[FAN_PERC]]);
	udp_buf[buf_len++] = '\n';
	
	char space = ' ';
	uint16_t crc16 = crc16calc(&space, CRC_INIT, 1);
	crc16 = crc16calc(&udp_buf[0], crc16, buf_len);
	char crc16_str[5] = "0";
	uint8_t crc16_len = uint_to_string(&crc16_str[0], crc16);
	
	char at_cmd[18] = "AT+CIPSEND=";
	uint8_t at_len = _strlen(at_cmd);
	at_len += uint_to_string(&at_cmd[at_len], buf_len + crc16_len);
	at_cmd[at_len -1] = '\r';
	at_cmd[at_len++] = '\n';
	
	sendATcmd(&at_cmd[0], at_len);
	delay_ms(1);
	sendATcmd(&crc16_str[0], crc16_len);
	sendATcmd(&udp_buf[0], buf_len);
	
}

void processData(uint8_t *recv_buf, uint8_t recv_len)
{
	ToggleTestPin1();
	if( recv_len > 0x20 ){
		
		ToggleTestPin2();
		switch(param_iter){
			case FAN_PERC:
				PresetDisp(recv_buf[param[param_iter]], 0);
				break;
			case SET_TEMP:
				PresetDisp(recv_buf[param[param_iter]], 0);
				break;
			case TEMP1:
				PresetDisp( (uint16_t) ((recv_buf[param[param_iter]] << 8) |  (recv_buf[param[param_iter] + 1]) ), 2);
				break;
			case TEMP2:
				PresetDisp( (uint16_t) ((recv_buf[param[param_iter]] << 8) |  (recv_buf[param[param_iter] + 1]) ), 2);
				break;
			case EXH_TEMP:
				PresetDisp( (uint16_t) ((recv_buf[param[param_iter]] << 8) |  (recv_buf[param[param_iter] + 1]) ), 2);
			default:
				break;
		}
		sendToESP(recv_buf, recv_len);
	}	
	uartSend((uint8_t *) &uart_respond[0], sizeof(uart_respond));
}

void TIM2_IRQHandler()
{

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
		if( delay_5ms_ticks >= 0 ){
			delay_5ms_ticks++;
		}
		DispMux();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

void TIM7_IRQHandler()
{

    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
		
		EXTI_ClearITPendingBit(EXTI_Line0);
		ToggleTestPin4();
		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0 )
		{
			NVIC_EnableIRQ(EXTI0_IRQn);
			TIM_Cmd(TIM7, DISABLE);
			ClearTestPin4();
		}
		
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}


void EXTI0_IRQHandler(void) 
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
	{
		NVIC_DisableIRQ(EXTI0_IRQn);
		
		if( param_iter >= sizeof(param) ){
			param_iter = 0;
		}else {
			param_iter++;
		}
		
		ToggleTestPin();
		EXTI_ClearITPendingBit(EXTI_Line0);
		TIM_Cmd(TIM7, ENABLE);
    }
}

void TIM7_Init(void)
{
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	//100ms
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 1999;
    TIM_TimeBaseInitStruct.TIM_Period = 4199;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM7, DISABLE);
}

void TIM2_Init()
{
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	//10ms
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 999;
    TIM_TimeBaseInitStruct.TIM_Period = 419;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}


void EXTIRQ_Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void initUart2(void)
{
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
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void initUart3(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	USART_InitTypeDef USART_InitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART3, &USART_InitStruct);
	USART_Cmd(USART3, ENABLE);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStruct);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
}

void initDispPins(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void initESP(void)
{
	const uint8_t estabUDPcmd[] = {'A', 'T', '+', 'C', 'I', 'P', 'S', 'T', 'A', 'R', 'T', '=', '"', 'U', 'D', 'P', '"', ',', '"', '1', '9', '2', '.', '1', '6', '8', \
								  	'.', '1', '.', '1', '0', '5', '"', ',', '8', '9', '0', '1', ',', '8', '0', '0', '1', '\r', '\n'};
	
	for(uint8_t i = 0; i < sizeof(estabUDPcmd); ++i){
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
		USART_SendData(USART3, estabUDPcmd[i]);
	}
	delay_ms(1);
	
}
int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();
	
	initDispPins();
	DispInit();
	
	EXTIRQ_Init();
	TIM7_Init();
	TIM2_Init();
	initUart2();
	initUart3();
	initESP();
	
	SetTestPin();
	
	for (;;){
		while( delay_5ms_ticks < 10 );
		delay_5ms_ticks= -1;
		processData(&rx_buf[0], frame_byte_rec);
		frame_byte_rec = 0;
	};
}
