#ifndef __GDISP_H__
#define __GDISP_H__

#include <stdint.h>

#define gdispXRes               (240u)
#define gdispYRes               (128u)
#define gdispCalibPointsNmb     (5u)

void gdispInit(void);
void gdispWriteText(char * text, uint8_t len, uint8_t colId, uint8_t rowId);
void gdispDrawItem(uint8_t *item, uint8_t ypos, uint8_t xpos, uint8_t yPixlen, uint8_t xPixlen);
void gdispDispCalibPoints(void);

#endif