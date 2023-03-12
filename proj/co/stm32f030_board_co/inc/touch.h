#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <stdint.h>

typedef struct{
    int8_t xCalibPos;
    int8_t yCalibPos;
}touchCalibPosType;

void touchInit(void);
void touchGetCalibratedPos(touchCalibPosType *calibratedPositions);
void touchCalibrate(void);

#endif