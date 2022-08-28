#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include "touch.h"

#define touchI2cAddr    (0x90u)

#define touchXAxisCmd   (0xC0u)
#define touchYAxisCmd   (0xD0u)
#define touchZ1Cmd      (0xE0u)
#define touchZ2Cmd      (0xF0u)
#define touchSleepCmd   (0x70u)

void touchInit(void)
{

}

void touchGetPox(uint16_t *xpos, uint16_t *ypos)
{

}