#include "gdisp.h"
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_spi.h>
#include "stdbool.h"

#define gdispPORT       GPIOB

#define gdispPinMOSI    GPIO_Pin_15
#define gdispPinMISO    GPIO_Pin_14
#define gdispPinCLK     GPIO_Pin_13
#define gdispPinCS      GPIO_Pin_12

#define gdispPinBLigth  GPIO_Pin_0

#define gdispSetCLK_High()  GPIO_SetBits(gdispPORT, gdispPinCLK);
#define gdispSetCLK_Low()   GPIO_ResetBits(gdispPORT, gdispPinCLK);
#define gdispSetCS_High()   do {\
                                GPIO_SetBits(gdispPORT, gdispPinMISO);\
                                GPIO_SetBits(gdispPORT, gdispPinCS);\
                            } while (0u);
#define gdispSetCS_Low()    do {\
                                GPIO_ResetBits(gdispPORT, gdispPinCS);\
                                GPIO_ResetBits(gdispPORT, gdispPinMISO);\
                            } while (0u);

#define gdispSetMOSI(boolean_val) do {\
    if(true == boolean_val) GPIO_SetBits(gdispPORT, gdispPinMOSI);\
    else GPIO_ResetBits(gdispPORT, gdispPinMOSI);\
    } while (0u);

static const uint8_t gdispInitBuf1[] = {0xe2, 0x25, 0x2b, 0xc4, 0xc8, 0x00, 0xa1, 0xd1, 0x89, 0xd6, 0xeb,
                                     0x81, 0x12, 0xb0, 0xa6, 0xf1, 0x7f, 0xf4, 0x00, 0xf5, 0x00, 0xf6,
                                     0x4f, 0xf7, 0x7f, 0xf8, 0xaf};
#if(0)
static const uint8_t gdispInitBuf2[] = {0x81, 0x1e};
static const uint8_t gdispInitBuf3[] = {0xf4, 0x00, 0xf5, 0x00, 0xf6, 0x27, 0xf7, 0x7f, 0x00, 0x10, 0x60, 0x70};
static const uint8_t gdispInitBuf4[] = {0xe2, 0x2f, 0x25, 0xea, 0x81, 0x4f, 0x40, 0x88, 0xaf};
#endif

static void gdispSend(uint8_t * buf, uint8_t len)
{
    for(uint8_t byteId = 0u; byteId < len; ++byteId)
    {
        gdispSetCS_Low();
        for(int8_t bitId = 7u; bitId > 0; --bitId)
        {
            bool bit = (buf[byteId] & (1u << bitId)) > 0u;
            gdispSetCLK_Low();
            gdispSetMOSI(bit);
            gdispSetCLK_High();
        }
        gdispSetCS_High();
        gdispSetMOSI((bool) 1u);
    }
    gdispSetCLK_Low();
}

static void gdispInitBacklight()
{
    GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = gdispPinBLigth;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gdispPORT, &GPIO_InitStruct);

    GPIO_ResetBits(gdispPORT, gdispPinBLigth);
}

static void gdispSPI_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = gdispPinMOSI | gdispPinCLK | gdispPinCS;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gdispPORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = gdispPinMISO;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(gdispPORT, &GPIO_InitStruct);

    GPIO_ResetBits(gdispPORT, gdispPinCLK);
    GPIO_SetBits(gdispPORT, gdispPinMOSI);
    GPIO_SetBits(gdispPORT, gdispPinMISO);
    GPIO_SetBits(gdispPORT, gdispPinCS);
}

extern void delay_ms(uint32_t ms);

void gdispInit(void)
{
    gdispSPI_Init();
    gdispInitBacklight();

    gdispSend((uint8_t *) &gdispInitBuf1[0], sizeof(gdispInitBuf1)/sizeof(gdispInitBuf1[0]));
    delay_ms(1);
    //gdispSend((uint8_t *) &gdispInitBuf2[0], sizeof(gdispInitBuf2)/sizeof(gdispInitBuf2[0]));
    //delay_ms(2);
    //gdispSend((uint8_t *) &gdispInitBuf3[0], sizeof(gdispInitBuf3)/sizeof(gdispInitBuf3[0]));
}