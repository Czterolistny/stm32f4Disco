#ifndef __MAIN_H__
#define __MAIN_H__

#define FAN_PERC 	0
#define SET_TEMP 	1
#define TEMP1 		2
#define TEMP2		3
#define EXH_TEMP	4

void delay_ms(uint32_t ms);

typedef struct{
	volatile uint16_t *param;
	const uint8_t paramNmb;
}CoParamType;

#endif