#include <stdbool.h>
#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "sregs.h"


#define SREG_DS             GPIO_Pin_14
#define SREG_CLK            GPIO_Pin_15
#define SREG_DS_CLK_PORT    GPIOC

#define SREG_OE             GPIO_Pin_5
#define SREG_RCLK           GPIO_Pin_8
#define SREG_OE_RCLK_PORT   GPIOB

#define SREG_OUTS           (16u)
#define SREG_MAX_OUTS_NMB   (1 << 15u)

#define gdispSetSregClk_High()      (SREG_DS_CLK_PORT->BSRR = SREG_CLK)
#define gdispSetSregClk_Low()       (SREG_DS_CLK_PORT->BRR = SREG_CLK)
#define gdispSetSregRClk_High()     (SREG_OE_RCLK_PORT->BSRR = SREG_RCLK)
#define gdispSetSregRClk_Low()      (SREG_OE_RCLK_PORT->BRR = SREG_RCLK)
#define gdispSetSregDs_High()       (SREG_DS_CLK_PORT->BSRR = SREG_DS)
#define gdispSetSregDs_Low()        (SREG_DS_CLK_PORT->BRR = SREG_DS)

static void sregRefresh(uint16_t outVal);

static uint16_t sregOutput = (SREGS_ESP_UART_EN_DEFAULT_STATE      << 15u)\
                             | (SREGS_ESP_DTR_RTS_EN_DEFAULT_STATE << 14u)\
                             | (SREGS_UART_BOOT_EN_DEFAULT_STATE   << 13u)\
                             | (SREGS_ESP_SW_UART_EN_DEFAULT_STATE << 12u)\
                             | (SREGS_ESP_RTS_DEFAULT_STATE        << 11u)\
                             | (SREGS_ESP_DTR_DEFAULT_STATE        << 10u)\
                             | (SREGS_SD_CS_DEFAULT_STATE          << 9u)\
                             | (SREGS_SREG1_OUT_DEFAULT_STATE      << 8u)\
                             | (SREGS_ESP_EN_DEFAULT_STATE         << 7u)\
                             | (SREGS_ESP_WAKE_DEFAULT_STATE       << 6u)\
                             | (SREGS_DIG_OUT2_DEFAULT_STATE       << 5u)\
                             | (SREGS_MCU_RST_DEFAULT_STATE        << 4u)\
                             | (SREGS_FLASH_CS_DEFAULT_STATE       << 3u)\
                             | (SREGS_BOOT0_DEFAULT_STATE          << 2u)\
                             | (SREGS_UNDEF1_DEFAULT_STATE         << 1u)\
                             | (SREGS_UNDEF2_DEFAULT_STATE         << 0u);

void sregsInit(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = SREG_DS | SREG_CLK;
    GPIO_Init(SREG_DS_CLK_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = SREG_OE | SREG_RCLK;
    GPIO_Init(SREG_OE_RCLK_PORT, &GPIO_InitStruct);

    /* Enable outputs */
    GPIO_WriteBit(SREG_OE_RCLK_PORT, SREG_OE, Bit_RESET);
    /* Write initial values to registers */
    sregRefresh(sregOutput);
}

static void sregRefresh(uint16_t outVal)
{
    for(uint8_t i = 0; i < SREG_OUTS; ++i)
    {
        gdispSetSregClk_Low();
        gdispSetSregRClk_Low();

        if( 0u != ((outVal >> i) & 0x01) )
        {
            gdispSetSregDs_High();
        }else
        {
            gdispSetSregDs_Low();
        }
        gdispSetSregClk_High();
        /* Prevent from uninteded pins toogling on SReg1 */
        if( 7u <= i )
        {
            gdispSetSregRClk_High();
        }
    }
}

void sregsGetOutput(uint16_t outputNmb, bool *state)
{
    if( outputNmb > SREG_MAX_OUTS_NMB )
    {
        return;
    }
    *state = (true == ((sregOutput & outputNmb) > 0u));
}

void sregsSetOutput(uint16_t outputNmb, bool state)
{
    bool actualState;
    if(outputNmb > SREG_MAX_OUTS_NMB)
    {
        return;
    }

    sregsGetOutput(outputNmb, &actualState);
    if( actualState != state )
    {
        if( true == state )
        {
            sregOutput |= outputNmb;
        }else
        {
            sregOutput &= ~(outputNmb);
        }
        sregRefresh(sregOutput);
    }
}