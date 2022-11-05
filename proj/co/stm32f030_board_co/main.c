#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_syscfg.h>
#include "sregs.h"
#include "debugPins.h"
#include "swuart.h"
#include "esp.h"
#include "gdisp.h"
#include "gdispFonts.h"
#include "../../common/common.h"
#include "main.h"
#include "i2c.h"
#include "flash.h"
#include "touch.h"

#define PARAM_FAN_PERC_IDX 		((uint8_t) 0x3Fu)
#define PARAM_SET_TEMP_IDX 		((uint8_t) 0x23u)
#define PARAM_TEMP1_IDX 		((uint8_t) 0x1Eu)
#define PARAM_TEMP2_IDX 		((uint8_t) 0x26u)
#define PARAM_EXH_TEMP_IDX 		((uint8_t) 0x42u)

#define PARAM_CNT		((uint8_t) 0x05u)

#define CO_CONST_RESP	{(uint8_t) 0x02u, (uint8_t) 0x26u, (uint8_t) 0xffu, (uint8_t) 0xf4u, (uint8_t) 0x16u, (uint8_t) 0xf9u, (uint8_t) 0x00u, \
						(uint8_t) 0x01u, (uint8_t) 0x16u, (uint8_t) 0xc2u, (uint8_t) 0x00u, (uint8_t) 0x00u, (uint8_t) 0x16u, (uint8_t) 0xf9u, \
						(uint8_t) 0x00u, (uint8_t) 0x00u, (uint8_t) 0x16u, (uint8_t) 0xf9u, (uint8_t) 0x00u, (uint8_t) 0x02u, (uint8_t) 0x16u, \
						(uint8_t) 0xc2u, (uint8_t) 0x00u, (uint8_t) 0x00u, (uint8_t) 0x16u, (uint8_t) 0xf9u, (uint8_t) 0x00u, (uint8_t) 0x00u, \
						(uint8_t) 0x02u, (uint8_t) 0x18u, (uint8_t) 0x2cu, (uint8_t) 0x11u}

#define CO_CONST_RESP_LEN	((uint8_t) 32u)

#define STR_FAN			"Fan: "
#define STR_SET_TEMP	"setTemp: "
#define STR_TEMP1		"Temp1: "
#define STR_TEMP2		"Temp2: "
#define STR_EXH_TEMP	"ExhTemp: "

#define getStrLen(str) (uint8_t)(sizeof(str)/sizeof(char))

#define getArrRef(arr) (&arr[0u])

#define NULL ((void*)0)

volatile static uint8_t *ptx_buf = NULL;

volatile uint8_t rx_cnt;
volatile uint8_t tx_cnt;

volatile uint32_t msTicks;

void delay_ms(uint32_t ms);
#if(0)
static void uartSend(uint8_t *tx_buf, uint8_t len);
#endif

volatile uint8_t swuartBuf[32];
volatile uint8_t swuartRxCnt;

typedef struct{
	volatile uint8_t uart_req[128u];
	const uint8_t uart_response[CO_CONST_RESP_LEN];
	const uint8_t response_len;
	const uint8_t param_idxs[PARAM_CNT];
	volatile bool req_ready;
	const CoParamType *mesure_param;
}CoControlCtx;

static volatile uint16_t gCoParam[PARAM_CNT];
const CoParamType CoParam = { .param = getArrRef(gCoParam), .paramNmb = sizeof(sizeof(gCoParam) / sizeof(gCoParam[0u]))};

CoControlCtx ControlCtx = { .param_idxs = {PARAM_FAN_PERC_IDX, PARAM_SET_TEMP_IDX, PARAM_TEMP1_IDX, PARAM_TEMP2_IDX, PARAM_EXH_TEMP_IDX}, \
							.uart_response = CO_CONST_RESP, .response_len = CO_CONST_RESP_LEN, .req_ready = false, .mesure_param = &CoParam};

typedef struct{
	volatile uint16_t const *val;
	struct{
		const char * const name;
		char len;
	};
	uint8_t xpos;
	uint8_t ypos;
}CoDispParam;

typedef struct{
	CoDispParam *dispParam;
	uint8_t paramCnt;
}CoDispCtx;

CoDispParam dispParam[PARAM_CNT] = {{.name = STR_FAN, .len = getStrLen(STR_FAN), .val = &gCoParam[0u], .xpos = 0u, .ypos = 10u},
						 {.name = STR_SET_TEMP, .len = getStrLen(STR_SET_TEMP), .val = &gCoParam[1u], .xpos = 0u, .ypos = 30u},
						 {.name = STR_TEMP1, .len = getStrLen(STR_TEMP1), .val = &gCoParam[2u], .xpos = 0u, .ypos = 50u},
						 {.name = STR_TEMP2, .len = getStrLen(STR_TEMP2), .val = &gCoParam[3u], .xpos = 0u, .ypos = 70u},
						 {.name = STR_EXH_TEMP, .len = getStrLen(STR_EXH_TEMP), .val = &gCoParam[4u], .xpos = 0u, .ypos = 90u}};

CoDispCtx dispCtx = {.dispParam = getArrRef(dispParam), .paramCnt = PARAM_CNT};

static void swuartRxByteComplete(uint8_t rxByte, uint8_t rxByteNmb)
{
	swuartBuf[swuartRxCnt++] = rxByte;
}
static void swuartTxComplete(uint16_t txByteNmb)
{

}
static void swuartRxComplete(uint8_t rxByteNmb)
{
	swuartSend((uint8_t*) getArrRef(swuartBuf), swuartRxCnt);
	swuartRxCnt = 0;
}

swUartConfigType *swConf = &swUartConfig;

void swuartTest(void)
{
	const uint8_t buf[] = "helloSW_Uart\n";

	swConf->swuartRxCompleteClb = &swuartRxComplete;
	swConf->swuartRxOneByteCompleteClb = &swuartRxByteComplete;
	swConf->swuartTxCompleteClb = &swuartTxComplete;

	swuartInit(swConf);
	swuartSend((uint8_t *)&buf[0], sizeof(buf)/sizeof(buf[0]));
	for(;;);
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
		USART_ClearFlag(USART1, USART_IT_RXNE);
		ControlCtx.uart_req[rx_cnt++] = (uint8_t) USART_ReceiveData(USART1);

		TIM_SetCounter(TIM3, (uint32_t) 0);
		if( TIM_GetITStatus(TIM3, TIM_IT_Update) == RESET )
		{
			TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		}

    }else if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
		static uint8_t txed_cnt;
		if(txed_cnt != tx_cnt)
		{
			USART_SendData(USART1, ControlCtx.uart_response[txed_cnt++]);
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
		ControlCtx.req_ready = true;
		TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

static void writeToDisp(CoDispCtx *dispCtx)
{
	char buf[8];
	uint8_t strLen;

	gdispFontSetFontType(Font_Times_New_Roman11x12);

	for(uint8_t ctx = 0u; ctx < dispCtx->paramCnt; ++ctx)
	{
		strLen = uint_to_string(getArrRef(buf), (uint16_t) *dispCtx->dispParam[ctx].val);
		gdispWriteText((char*) dispCtx->dispParam[ctx].name, dispCtx->dispParam[ctx].len,\
		 dispCtx->dispParam[ctx].ypos, dispCtx->dispParam[ctx].xpos);
		gdispWriteText(getArrRef(buf), strLen, dispCtx->dispParam[ctx].ypos, 20u);
	}
}

static void uartSendToCO(CoControlCtx *ctx)
{
	ptx_buf = (uint8_t*) ctx->uart_response;
	tx_cnt = ctx->response_len;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

#if(0)
static void uartSend(uint8_t *tx_buf, uint8_t len)
{
	for(uint8_t i = 0; i < len; ++i)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, tx_buf[i]);
	}
}
#endif

static void updateCoParam(CoControlCtx *ctx)
{	
	uint16_t *param = (uint16_t*) ctx->mesure_param->param;
	uint8_t *req_buf = (uint8_t*) getArrRef(ctx->uart_req);
	param[0u] = req_buf[ctx->param_idxs[SET_TEMP]];
	param[1u] = ((req_buf[ctx->param_idxs[TEMP1]] << 8u) | req_buf[ctx->param_idxs[TEMP1] + 1u]) / 10u;
	param[2u] = ((req_buf[ctx->param_idxs[TEMP2]] << 8u) | req_buf[ctx->param_idxs[TEMP2] + 1u]) / 10u;
	param[3u] = ((req_buf[ctx->param_idxs[EXH_TEMP]] << 8u) | req_buf[ctx->param_idxs[EXH_TEMP] + 1u]) / 10u;
	param[4u] = req_buf[ctx->param_idxs[FAN_PERC]];
}

static void processData(CoControlCtx *ctx, uint8_t recv_len)
{
	if( recv_len > 0x20 )
	{
		updateCoParam(ctx);
		espWrite(ctx->mesure_param);

		writeToDisp(&dispCtx);
	}
	uartSendToCO(ctx);
}

static void TIM3_Init()
{
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM3EN, ENABLE);

	/* 100ms Timer */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 999;
    TIM_TimeBaseInitStruct.TIM_Period = 4199;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM3, ENABLE);
}

static void InitUsart1(void)
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

void initESP(void)
{
	static espConfig esp_conf;
	esp_conf.espWriteATCommand = &swuartSend;
	espInit(&esp_conf);
}

__attribute__((noreturn)) void runCoProcEngine(CoControlCtx *ctx) 
{
	for (;;){
		if( true == ctx->req_ready )
		{
			processData(ctx, rx_cnt);
			rx_cnt = 0;
			ctx->req_ready = false;
		}
	};
}

int main()
{
	SystemInit();
    SysTick_Config(SystemCoreClock / 1000);

	initTestPin();

	/* SW uart test - never return function */
	/* swuartTest(); */

	InitUsart1();
	TIM3_Init();

	sregsInit();
	initESP();
	gdispInit();
	
	i2cInit();
	flashInit();

	touchInit();
	
	/* never returns */
	runCoProcEngine(&ControlCtx);
}
