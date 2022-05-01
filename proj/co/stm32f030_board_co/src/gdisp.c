#include "gdisp.h"
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_spi.h>
#include "stdbool.h"

#define gdispXRes       (240u)
#define gdispYRes       (128u)

#define gdispPORT       GPIOB

#define gdispPinMOSI    GPIO_Pin_15
#define gdispPinMISO    GPIO_Pin_14
#define gdispPinCLK     GPIO_Pin_13
#define gdispPinCS      GPIO_Pin_12

#define gdispPinBLigth  GPIO_Pin_0

#define gdispSetMISO_High() GPIO_SetBits(gdispPORT, gdispPinMISO);
#define gdispSetMISO_Low()  GPIO_ResetBits(gdispPORT, gdispPinMISO);

#define gdispSetCLK_High()  GPIO_SetBits(gdispPORT, gdispPinCLK);
#define gdispSetCLK_Low()   GPIO_ResetBits(gdispPORT, gdispPinCLK);

#define gdispSetCS_High()   GPIO_SetBits(gdispPORT, gdispPinCS);
#define gdispSetCS_Low()    GPIO_ResetBits(gdispPORT, gdispPinCS);

#define gdispSetMISO_CS_High()   do {\
                                    gdispSetMISO_High();\
                                    gdispSetCS_High();\
                                } while (0u);

#define gdispSetCS_MISO_Low()    do {\
                                    gdispSetCS_Low();\
                                    gdispSetMISO_Low();\
                                } while (0u);

#define gdispSetMOSI(boolean_val) do {\
    if(true == boolean_val) GPIO_SetBits(gdispPORT, gdispPinMOSI);\
    else GPIO_ResetBits(gdispPORT, gdispPinMOSI);\
    } while (0u);

static const uint8_t gdispInitBuf[] = {0xe2, 0x25, 0x2b, 0xc4, 0xc8, 0x00, 0xa1, 0xd1, 0x89, 0xd6, 0xeb,
                                     0x81, 0x12, 0xb0, 0xa6, 0xf1, 0x7f, 0xf4, 0x00, 0xf5, 0x00, 0xf6,
                                     0x4f, 0xf7, 0x7f, 0xf8, 0xaf, 0x81, 0x1e, 0xf4, 0x00, 0xf5, 0x00, 
                                     0xf6, 0x4f, 0xf7, 0x7f, 0x00, 0x10, 0x60, 0x70};
#if(0)
static const uint8_t gdispInitBuf1[] = {0xe2, 0x25, 0x2b, 0xc4, 0xc8, 0x00, 0xa1, 0xd1, 0x89, 0xd6, 0xeb,
                                     0x81, 0x12, 0xb0, 0xa6, 0xf1, 0x7f, 0xf4, 0x00, 0xf5, 0x00, 0xf6,
                                     0x4f, 0xf7, 0x7f, 0xf8, 0xaf};

static const uint8_t gdispInitBuf2[] = {0x81, 0x1e};
static const uint8_t gdispInitBuf3[] = {0xf4, 0x00, 0xf5, 0x00, 0xf6, 0x4f, 0xf7, 0x7f, 0x00, 0x10, 0x60, 0x70};
static const uint8_t gdispInitBuf4[] = {0xe2, 0x2f, 0x25, 0xea, 0x81, 0x4f, 0x40, 0x88, 0xaf};
#endif

inline static void gdispSendByte(uint8_t byte);

static void gdispSendByte(uint8_t byte)
{
    for(int8_t bitId = 7; bitId >= 0; --bitId)
    {
        bool bit = (byte & (1u << bitId)) > 0u;
        gdispSetCLK_Low();
        gdispSetMOSI(bit);
        gdispSetCLK_High();
    }
}
static void gdispSendCmd(uint8_t * buf, uint8_t len)
{
    for(uint8_t byteId = 0u; byteId < len; ++byteId)
    {
        gdispSetCS_MISO_Low();
        gdispSendByte(buf[byteId]);
        gdispSetMISO_CS_High();
        gdispSetMOSI((bool) 1u);
    }
    gdispSetCLK_Low();
}

static void gdispSendData(uint8_t * buf, uint8_t len)
{
    gdispSetCS_High();
    for(uint8_t byteId = 0u; byteId < len; ++byteId)
    {
        gdispSetMISO_Low();
        gdispSendByte(buf[byteId]);
        gdispSetMISO_High();
        gdispSetMOSI((bool) 1u);
    }
    gdispSetCLK_Low();
}

static void gdispInitBacklight()
{
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);

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
    /* Prevent low-state transition during init */
    GPIO_SetBits(gdispPORT, gdispPinCS);
    GPIO_SetBits(gdispPORT, gdispPinMOSI);

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = gdispPinMOSI | gdispPinCLK | gdispPinCS;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gdispPORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = gdispPinMISO;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(gdispPORT, &GPIO_InitStruct);

    GPIO_ResetBits(gdispPORT, gdispPinCLK);
    GPIO_SetBits(gdispPORT, gdispPinMISO);
}

extern void delay_ms(uint32_t ms);

static void gdispSetPixBuf(uint8_t pix, uint8_t * buf)
{
    buf[0] = 0u; buf[1] = 0u;

    if( 0u != (0x01u & pix) )
    {
        buf[0] |= 0xF0u;
    }
    if( 0u != (0x02u & pix) )
    {
        buf[1] |= 0x80u;
        buf[0] |= 0x07u;
    }
    if( 0u != (0x04u & pix) )
    {
        buf[1] |= 0x1Fu;
    }
}

static void gdispClearDisp(void)
{
    uint8_t x = 0u;
    for(uint16_t i = 0u; i < (gdispXRes * gdispYRes) / 3u; ++i)
    {
        gdispSendData(&x, 1u);
        gdispSendData(&x, 1u);
    }
}

void gdispTest(void)
{
    uint8_t buf[2];
    for(uint16_t i = 0x00u; i < 0xa4; ++i)
    {
        gdispSetPixBuf(1u, &buf[0]);
        gdispSendData(&buf[0], 2u);
    }
}

void gdispInit(void)
{
    gdispSPI_Init();
    gdispInitBacklight();

    delay_ms(10);
    gdispSendCmd((uint8_t *) &gdispInitBuf[0], sizeof(gdispInitBuf)/sizeof(gdispInitBuf[0]));
    
    gdispClearDisp();
    gdispTest();
}