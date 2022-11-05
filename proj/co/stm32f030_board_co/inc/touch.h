#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <stdint.h>

void touchInit(void);
void touchGetPox(uint16_t *xpos, uint16_t *ypos);

#endif