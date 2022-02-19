#ifndef __LED_DISP_H__
#define __LED_DISP_H__

#define DOT 1
#include "led_disp_port.h"

/* Initialize display with "----" characters. Not mandatory */
void DispInit(void);

/* Must be called periodically to preserve multiplexing */
void DispMux(void);

/* Set display values with dot */
void PresetDisp(int16_t val, uint8_t dot);

#endif
