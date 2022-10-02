#ifndef __GDISP_H__
#define __GDISP_H__

#include <stdint.h>

void gdispInit(void);
void gdispWriteText(char * text, uint8_t len, uint8_t colId, uint8_t rowId);
void gdispDispCalibPoints(void);

#endif