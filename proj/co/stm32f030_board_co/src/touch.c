#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include "touch.h"
#include "i2c.h"
#include "gdisp.h"
#include "gdispFonts.h"
#include "../../../common/common.h"
#include "stdbool.h"

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

typedef struct{
    uint8_t xAdcRow;
    uint8_t yAdcRow;
}touchRowPoints;

typedef struct{
    uint16_t xCoeff;
    uint16_t yCoeff;
    uint16_t zCoeff;
}touchCalibCoeffs;

touchCalibCoeffs touchCalibrateionCoeffs;
touchCalibPosType touchCalibratedPos;

static void touchGetPox(touchRowPoints *touchPts)
{
    touchPts->xAdcRow = touchReadReg(touchYpXn1Cmd);
    touchPts->yAdcRow = touchReadReg(touchXAxisCmd);
}

static void touchLog(touchRowPoints *touchPts)
{
    /* Write to disp and confirm that point 'x' is saved
    and to Tap a next calibration point */
}

static void touchDoCalibration(touchRowPoints *touchPts)
{
    bool tapNotification = false;
    uint8_t calibPoint = 0u;
    char * calibDoneStr = "Calib done..\n";

    do{
        if( true == tapNotification )
        {
            delay_ms(20);
            touchGetPos(&touchPts[calibPoint]);
            tapNotification = false;
            touchLog(&touchPts[calibPoint]);
            calibPoint++;
        }

    }while( gdispCalibPointsNmb > calibPoint );

    gdispWriteText(calibDoneStr, _strlen(calibDoneStr), gdispXRes / 3u, gdispYRes + 10u);
}

static void touchCalcCalibCoeff(touchRowPoints *touchPts)
{
    /* some calibration calculations */
}

static void touchSaveCalibCoeff(touchCalibCoeffs *calibCoeffs)
{
    /* store in none-volatile memory */
}

static void touchReadCalibCoeff(touchCalibCoeffs *calibCoeffs)
{
    /* after power-up */
}

void touchGetCalibratedPos(touchCalibPosType * points)
{
    /* return position by display pixels (calibrated) */
}

void touchCalibrate(void)
{
    char * calibInfoStr = "Calibration Points\n";
    touchRowPoints touchPts[gdispCalibPointsNmb];

    gdispFontSetFontType(Font_Times_New_Roman11x12);
    gdispDispCalibPoints();

    gdispWriteText(calibInfoStr, _strlen(calibInfoStr), gdispXRes / 3u, gdispYRes - 10u);

    touchDoCalibration(&touchPts);
    touchCalcCalibCoeff(&touchPts);
}

void touchInit(void)
{
    i2cInit();
    touchReadCalibCoeff(&touchCalibratedPos);
}