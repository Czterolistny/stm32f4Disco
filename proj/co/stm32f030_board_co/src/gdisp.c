#include "gdisp.h"
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_spi.h>
#include "stdbool.h"
#include "gdispFonts.h"

#define MSB_FIRST
#ifdef MSB_FIRST
    #define gdispPixMask                    (0xE0u)
    #define gdispIncrPixIter(pixIter)       (pixIter >> gdispPixNmbInWord)
    #define gdispFirstPixIdx                (4u)
    #define gdispThirdPixIdx                (1u)
    #define gdispMaxValPixShiftByte         (gdispPixMask >> 5u)
#else
    #define gdispPixMask                    (0x07u)
    #define gdispIncrPixIter(pixIter)       (pixIter << gdispPixNmbInWord)
    #define gdispFirstPixIdx                (1u)
    #define gdispThirdPixIdx                (4u)
    #define gdispMaxValPixShiftByte         (gdispPixMask << 5u)
#endif

//#define BIG_ENDIAN_ORDER

#define gdispSecondPixIdx       (2u)

#define gdispPix0Shift          (3u)
#define gdispPix1Shift          (0u)
#define gdispPix2Shift          (8u)

#define gdispPix0Mask           (uint16_t)(31u << gdispPix0Shift)
#define gdispPix1Mask           (uint16_t)(0xC007u << gdispPix1Shift)
#define gdispPix2Mask           (uint16_t)(31u << gdispPix2Shift)

#define gdispXRes               (240u)
#define gdispYRes               (128u)

#define gdispPixNmbInWord       (3u)

#define gdispPORT           GPIOB
#define gdispPinMOSI        GPIO_Pin_15
#define gdispPinMISO        GPIO_Pin_14
#define gdispPinCLK         GPIO_Pin_13
#define gdispPinCS          GPIO_Pin_12

#define gdispPinBLigth      GPIO_Pin_0

#define gdispSetMISO_High() gdispPORT->BSRR = gdispPinMISO;
#define gdispSetMISO_Low()  gdispPORT->BRR = gdispPinMISO;

#define gdispSetMOSI_High() gdispPORT->BSRR = gdispPinMOSI;
#define gdispSetMOSI_Low()  gdispPORT->BRR = gdispPinMOSI;

#define gdispSetCLK_High()  gdispPORT->BSRR = gdispPinCLK;
#define gdispSetCLK_Low()   gdispPORT->BRR = gdispPinCLK;

#define gdispSetCS_High()   gdispPORT->BSRR = gdispPinCS;
#define gdispSetCS_Low()    gdispPORT->BRR = gdispPinCS;

#define gdispSetMISO_CS_High()   do {\
                                    gdispSetMISO_High();\
                                    gdispSetCS_High();\
                                } while (0u);

#define gdispSetCS_MISO_Low()    do {\
                                    gdispSetCS_Low();\
                                    gdispSetMISO_Low();\
                                } while (0u);

#define gdispSetMOSI(boolean_val) do {\
    if(true == boolean_val) \
        gdispSetMOSI_High()\
    else \
        gdispSetMOSI_Low()\
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
inline static void gdispMake2ByteBuf(uint8_t *buf, uint16_t dispData);

void gdispSetPos(uint8_t row, uint8_t col);
void gdispSendData(uint8_t * buf, uint8_t len);

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

static void gdispSendRawData(uint8_t * buf, uint8_t len)
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

static void gdispMake2ByteBuf(uint8_t *buf, uint16_t dispData)
{
#ifdef BIG_ENDIAN_ORDER
    buf[1] = (uint8_t)(dispData & 0xFFu);
    buf[0] = (uint8_t)((dispData & 0xFF00u) >> 8u);
#else
    buf[0] = (uint8_t)(dispData & 0xFFu);
    buf[1] = (uint8_t)((dispData & 0xFF00u) >> 8u);
#endif
}

static void gdispSetPixBuf(uint8_t pix, uint16_t * dispData)
{
    *dispData = (gdispFirstPixIdx & pix)? gdispPix0Mask: 0u;
    *dispData |= (gdispSecondPixIdx & pix)? gdispPix1Mask: 0u;
    *dispData |= (gdispThirdPixIdx & pix)? gdispPix2Mask: 0u;
}

static void gdispSetGrayScale(uint8_t pix0, uint8_t pix1, uint8_t pix2, uint16_t *dispData)
{
    *dispData = (uint16_t) (pix0 << gdispPix0Shift);
    *dispData |= (uint16_t) (( ((pix1 & 0x1Cu) >> 2u) | ((pix1 & 0x03u) << 14u)) << gdispPix1Shift);
    *dispData |= (uint16_t) (pix2 << gdispPix2Shift);
}

static void gdispClearDisp(void)
{
    uint8_t x = 0u;
    for(uint16_t i = 0u; i < (gdispXRes * gdispYRes) / 3u; ++i)
    {
        gdispSendRawData(&x, 1u);
        gdispSendRawData(&x, 1u);
    }
}

void gdispSetRow(uint8_t row)
{
    uint8_t cmd[] = {0x60u, 0x70u};
    if( gdispYRes > row )
    {   
        cmd[0] += (row % 0x10u);
        cmd[1] += (row / 0x10u);
        gdispSendCmd(&cmd[0], 2u);
    }
}

void gdispSetCol(uint8_t col)
{
    uint8_t cmd[] = {0x00, 0x10};
    if( (gdispXRes / 3u) > col )
    {   
        cmd[0] += (col % 0x10u);
        cmd[1] += (col / 0x10u);
        gdispSendCmd(&cmd[0], 2u);
    }
}

void gdispSetPos(uint8_t row, uint8_t col)
{
    gdispSetCol(col);
    gdispSetRow(row);
}

void gdispSendDataU16(uint16_t data)
{
    uint8_t buf[2];
    buf[1] = (uint8_t)(data & 0xFFu);
    buf[0] = (uint8_t)((data & 0xFF00u) >> 8u);
    gdispSendData(&buf[0], 2u);
}

void gdispSendData(uint8_t * buf, uint8_t len)
{
    uint16_t pixIter = gdispPixMask;
    uint8_t pixIterShiftCnt = 0u;
    uint8_t pixIterRemaind;
    uint8_t dataBuf[2];
    uint16_t dispData;
    bool byteUnProcessed = true;

    for(uint8_t byteIdx = 0u; byteIdx < len; ++byteIdx)
    {
        for(; true == byteUnProcessed ;)
        {
            byteUnProcessed = false;
            uint8_t pixVal;
        #ifdef MSB_FIRST
            if( 5u > pixIterShiftCnt )
                pixVal = (buf[byteIdx] & (uint8_t)pixIter) >> (5u - pixIterShiftCnt);
            else
                pixVal = (buf[byteIdx] & (uint8_t)pixIter) << (pixIterShiftCnt - 5u);
            
            if( (uint16_t)0x07u > pixIter )
        #else
            pixVal = (buf[byteIdx] & (uint8_t)pixIter) >> pixIterShiftCnt;
            if( (uint16_t)0xFFu < pixIter )
        #endif
            {
                if( len > (byteIdx + 1u) )
                {
                #ifdef MSB_FIRST
                    uint8_t innerShift = (pixIter & 1u) + ((pixIter >> 1u) & 1u);
                    pixIterRemaind = gdispPixNmbInWord - innerShift;
                    pixVal |= (buf[byteIdx + 1u] & (uint8_t)(gdispPixMask << innerShift)) >> (5u + innerShift);
                    pixIter = gdispPixMask >> pixIterRemaind;
                #else
                    pixIter = pixIter >> 8u;
                    uint8_t innerShift = (pixIter & 1u) + ((pixIter >> 1u) & 1u);
                    pixIterRemaind = gdispPixNmbInWord - innerShift;
                    pixVal |= ((buf[byteIdx + 1u] & (uint8_t)pixIter) << pixIterRemaind);
                    pixIter = gdispPixMask << innerShift;
                #endif
                    pixIterShiftCnt = pixIterShiftCnt - 5u;
                }
            }
            else if( gdispMaxValPixShiftByte == pixIter )
            {
                pixIter = gdispPixMask;
                pixIterShiftCnt = 0u;
            }
            else
            {
                pixIter = gdispIncrPixIter(pixIter);
                pixIterShiftCnt += gdispPixNmbInWord;
                byteUnProcessed = true;
            }
            
            gdispSetPixBuf((uint8_t)pixVal, &dispData);
            gdispMake2ByteBuf(&dataBuf[0u], dispData);
            gdispSendRawData(&dataBuf[0u], 2u);
        }
        byteUnProcessed = true;
    }
}

void gdispWriteText(char * text, uint8_t len, uint8_t rowIdx, uint8_t colIdx)
{
    uint16_t dispData = 0u;
    uint8_t signSpan;
    FontStatus status;
    uint8_t tmpRowIdx = rowIdx;
    uint8_t tmpColIdx = colIdx;

    gdispSetPos(rowIdx, colIdx);
    for(uint8_t i = 0u; i < len; ++i)
    {
        tmpRowIdx = rowIdx;
        status = FONT_UNKNOWN;
        signSpan = gdispFontGetSignSpan(text[i]);
        while( FONT_COMPLETED != status )
        {
            status = gdispFontsGetFontByte(text[i], &dispData);
            if( FONT_NEXT_COLUMN == status )
            {
                gdispSetPos(tmpRowIdx, tmpColIdx);
            }else if( FONT_NEXT_ROW == status )
            {
                gdispSetPos(tmpRowIdx, tmpColIdx + (signSpan / 16u) * 5u);
                tmpRowIdx++;
            }
            gdispSendDataU16(dispData);
        }
        tmpColIdx += 1u + signSpan / 3u;
        gdispSetPos(rowIdx, tmpColIdx);
    }
}

void gdispInit(void)
{
    gdispSPI_Init();
    gdispInitBacklight();

    delay_ms(10);
    gdispSendCmd((uint8_t *) &gdispInitBuf[0], sizeof(gdispInitBuf)/sizeof(gdispInitBuf[0]));
    
    gdispClearDisp();

    //char str[] = "Makis Wapis 23059";
    //gdispFontSetFontType(Font_Times_New_Roman11x12);
    //gdispWriteText(&str[0], 21u, 0u, 0u);

    //gdispFontSetFontType(Font_Times_New_Roman23x22);
    //gdispWriteText(&str[0], 16u, 10u, 0u);
}