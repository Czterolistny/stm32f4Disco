#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_syscfg.h>
#include <stdbool.h>
#include "swuart.h"
#include "../debugPins.h"

#define NULL                    ((void*) 0u)
#define SWUART_USE_PARITY_BITS  (false)

#define swuartEXTSource EXTI4_15_IRQn
#define swuartEXTLine   EXTI_Line11
#define swuartTimer     TIM14
#define swuartTxPin     GPIO_Pin_4
#define swuartRxPin     GPIO_Pin_11
#define swuartGPIO      GPIOA

#define swuartSetTxLine(boolean_val) do {\
    if(true == boolean_val) GPIO_WriteBit(swuartGPIO, swuartTxPin, Bit_SET);\
    else GPIO_WriteBit(swuartGPIO, swuartTxPin, Bit_RESET);\
    } while (0u);

#define swuartReadtxLine()  GPIO_ReadInputDataBit(swuartGPIO, swuartRxPin)
#define swuartIsTxMode()    (0u != swuartTxLen)

#define swuartTimerPresc          (4u)
#define swuartBitPeriodTimVal     (125u)
#define swuartPeriodCorrection    (swuartTimerPresc * 15u)
#define swuartHalfBitPeriodTimVal ((swuartBitPeriodTimVal / 2u) + swuartPeriodCorrection)
#define swuartTxBufSize           (32u)
#define swuartMaxBitInByteShift   (7u)
#define swuartBitInFrame          (10u)
#define swuartStartBitIdx         (0u)

volatile static uint8_t swuartTxBuf[swuartTxBufSize];
volatile static uint8_t swuartTxLen;
volatile static uint8_t swuartRxBuf;
volatile static uint8_t swuartRxLen;
volatile static uint8_t swuartTxByteIdx;
volatile static uint8_t swuartTxBitIdx;
volatile static uint8_t swuartRxBitCnt;
volatile static uint8_t swuartBitInFrameIdx;

static inline void swuartPostTxAction(void);
static inline void swuartInitTxAction(void);
static inline void swuartInitRxAction(void);
static inline void swuartPostRxAction(void);
static inline void swuartRxIdleAction(void);

swuartTxCompleteCallb swuartTxComplete = NULL;
swuartRxByteCompleteCallb swuartRxByteComplete = NULL;
swuartRxCompleteCallb swuartRxComplete = NULL;

typedef union
{
    struct
    {
        uint16_t startBit :1;
        uint16_t data :8;
    #if(true == SWUART_USE_PARITY_BITS)
        uint16_t parity :1;
    #endif
        uint16_t stopBits :1;
    };
    uint16_t frame;
}swuartFrame_t;
swuartFrame_t swuartFrame = {.startBit = 0u, .stopBits = 1u};

static void swuartInitTxAction(void)
{
    swuartFrame.startBit = 0u;
    swuartFrame.stopBits = 1u;
    /* uininteded interrupt occures, so the below */
    swuartBitInFrameIdx = 0u;
    swuartFrame.data = swuartTxBuf[0u];
    NVIC_DisableIRQ(swuartEXTSource);
    TIM_Cmd(swuartTimer, ENABLE);
}

static void swuartPostTxAction(void)
{
    swuartTxLen = 0u;
    TIM_Cmd(swuartTimer, DISABLE);
    TIM_SetCounter(swuartTimer, (uint32_t) 0u);
    NVIC_EnableIRQ(swuartEXTSource);
    
    if( NULL != swuartTxComplete )
    {
        swuartTxComplete(0u);
    }
}

static void swuartInitRxAction(void)
{
    TIM_SetCounter(swuartTimer, (uint32_t) swuartHalfBitPeriodTimVal);
	TIM_Cmd(swuartTimer, ENABLE);
    NVIC_DisableIRQ(swuartEXTSource);
    swuartFrame.frame = 0u;
    swuartBitInFrameIdx = 0u;
}

static void swuartPostRxAction(void)
{
    EXTI_ClearITPendingBit(swuartEXTLine);
    NVIC_EnableIRQ(swuartEXTSource);

    if( NULL != swuartRxByteComplete )
    {
        swuartRxByteComplete(swuartFrame.data, 0u);
    }
}

static void swuartRxIdleAction(void)
{
    TIM_Cmd(swuartTimer, DISABLE);
    TIM_SetCounter(swuartTimer, (uint32_t) 0u);
    if( NULL != swuartRxComplete )
    {
        swuartRxComplete(0u);
    }
}

void TIM14_IRQHandler()
{
    if (TIM_GetITStatus(swuartTimer, TIM_IT_Update) != RESET)
    {
        if( true == swuartIsTxMode() )
        {
            if( swuartTxLen != swuartTxByteIdx )
            {
    	        swuartSetTxLine( ((swuartFrame.frame & (1 << swuartBitInFrameIdx)) > 0u) );
                if( (swuartBitInFrame - 1u) == swuartBitInFrameIdx )
                {
                    swuartBitInFrameIdx = 0u;
                    swuartTxByteIdx++;
                    swuartFrame.data = swuartTxBuf[swuartTxByteIdx];
                }else{
                    swuartBitInFrameIdx++;
                }
            }else
            {
                swuartTxByteIdx = 0;
                swuartPostTxAction();
            }
        }else
        {
            swuartFrame.frame |= (uint16_t)( swuartReadtxLine() << swuartBitInFrameIdx );
            if( (swuartBitInFrame - 1u) == swuartBitInFrameIdx )
            {
                swuartBitInFrameIdx++;
                swuartPostRxAction();
            }else if( swuartBitInFrame == swuartBitInFrameIdx )
            {
                swuartBitInFrameIdx = 0u;
                swuartRxIdleAction();
            }else{
                swuartBitInFrameIdx++;
            }
        }
    	TIM_ClearITPendingBit(swuartTimer, TIM_IT_Update);
    }
}

void EXTI4_15_IRQHandler() 
{
    if (EXTI_GetITStatus(swuartEXTLine) != RESET) 
	{
        swuartInitRxAction();
    }
}

static void swuartTimerInit(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM14EN, ENABLE);

	/* 1/(115200) Timer */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = swuartTimerPresc;
    TIM_TimeBaseInitStruct.TIM_Period = (swuartBitPeriodTimVal - 1);
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(swuartTimer, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM14_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(swuartTimer, TIM_IT_Update, ENABLE);
    TIM_Cmd(swuartTimer, DISABLE);
}

static void swuartPinsInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);

    /* To preserve GND-pulse after initialization */
    GPIO_WriteBit(swuartGPIO, swuartTxPin, Bit_SET);

    GPIO_InitStruct.GPIO_Pin = swuartTxPin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(swuartGPIO, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = swuartRxPin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(swuartGPIO, &GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource11);

	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = swuartEXTLine;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
	
    NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = swuartEXTSource;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void swuartSend(uint8_t *buf, uint8_t len)
{
    if(swuartTxBufSize >= len)
    {
        for(uint8_t i = 0; i < len; ++i)
        {
            swuartTxBuf[i] = buf[i];
        }
        swuartTxLen = len;
        swuartInitTxAction();
    }
    else
    {
        return;
    }
}

void swuartInit(void)
{
    swuartPinsInit();
    swuartTimerInit();
}

void swuartInitClb(swuartTxCompleteCallb txClb, swuartRxByteCompleteCallb rxByteClb,
            swuartRxCompleteCallb rxClb)
{
    swuartTxComplete = txClb;
    swuartRxByteComplete = rxByteClb;
    swuartRxComplete = rxClb;
}