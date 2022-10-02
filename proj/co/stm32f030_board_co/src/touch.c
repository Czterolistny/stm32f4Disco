#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include "touch.h"
#include "i2c.h"
#include "gdisp.h"
#include "../../../common/common.h"

#define touchI2cAddr        (0x90u)

#define touchXpXnCmd        (0x80u)
#define touchYpYnCmd        (0x90u)
#define touchYpXn1Cmd       (0xA0u)
#define touchYpXn2Cmd       (0xB0u)

#define touchXAxisCmd       (0xC0u)
#define touchYAxisCmd       (0xD0u)
#define touchZ1Cmd          (0xE0u)
#define touchZ2Cmd          (0xF0u)
#define touchSleepCmd       (0x70u)

#define touchBufSize        (0x02u)
static uint8_t g_touchBuf[touchBufSize];

#define touchSetBuf(cmd)    ({ \
                            do {\
                                g_touchBuf[0u] = touchI2cAddr;\
                                g_touchBuf[1u] = cmd;\
                            } while (0u); \
                            g_touchBuf; \
                            })
#define touchGetADC()       ((uint16_t) ((g_touchBuf[0u] << 4u) | (g_touchBuf[1u] >> 4u)))

#define touchReadReg(cmd)   ({ \
                            do { \
                                i2cWrite(touchSetBuf(cmd), touchBufSize); \
                                i2cRead(g_touchBuf, 0u, touchBufSize); \
                            } while (0u); \
                            touchGetADC(); \
                            })

void touchTest(void);

void touchInit(void)
{
    i2cInit();
    touchTest();
}

void touchGetPox(uint16_t *xpos, uint16_t *ypos)
{
    *xpos = touchReadReg(touchYpXn1Cmd);
    *ypos = touchReadReg(touchXAxisCmd);
}

void touchTest(void)
{
    char s[5];
    uint8_t len;
    uint16_t yVal = 0u, xVal = 0u;
	for(;;)
	{
		touchGetPox(&yVal, &xVal);
        len = uint_to_string(&s[0], yVal);
        gdispWriteText(&s[0u], len, 10u, 20u);

        len = uint_to_string(&s[0], xVal);
        gdispWriteText(&s[0u], len, 10u, 30u);
	}
}