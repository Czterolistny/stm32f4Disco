/** Platform dependent 7-segment display port definition **/

#ifndef __LED_DISP_PORT_H__
#define __LED_DISP_PORT_H__

#include <stm32f4xx.h>

#define OUT_PORT GPIOB
#define MUX_PORT GPIOC

#define OUT_PORT_PIN0 0
#define MUX_PORT_PIN0 0

#define DISP_SEG_MASK 0xFFFF - (0xFF << OUT_PORT_PIN0)
#define DISP_MUX_MASK 0xFFFF - (0xFF << MUX_PORT_PIN0)

#define SET_OUT_PORT(x)	OUT_PORT->ODR = (OUT_PORT->ODR & 0xFF00) | x
#define SET_MUX_PORT(x)	MUX_PORT->ODR = (MUX_PORT->ODR & 0xFFF0) | (1 << x)

#endif
