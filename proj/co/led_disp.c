#include <stdint.h>
#include <stdbool.h>
#include "led_disp.h"
#include "led_disp_port.h"

#define DIG_NUMBER 4
#define DOT_BIT ~(1 << 7)

#define NO_VAL 0xBF
#define DIMM_VAL 0xFF
#define MINUS_SIGN 0xBF

#define DIMM_DIGIT 1

#if DIMM_DIGIT
	#define SIGNED_SIGN 1
#endif

#define SET_PORT() NULL

/* 7 segment codes 0-9 */
const uint8_t Seg7_code[] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90 };

static uint8_t Seg7[DIG_NUMBER];

void DispInit()
{
	uint8_t i;
	for(i = 0; i < DIG_NUMBER; i++)
	{
		Seg7[i] = NO_VAL;
	}
}

void DispMux()
{
	static uint8_t DigNmb;

	SET_OUT_PORT(Seg7[DigNmb]);
	SET_MUX_PORT(DigNmb);

	DigNmb++;
	
	if(DigNmb >= DIG_NUMBER)
	{
		DigNmb = 0;
	}	
}

void PresetDisp(int16_t val, uint8_t dot)
{
	uint8_t dig_val;
	bool minus_sign = false;
	
	if( val < 0 )
	{
		/* Convert to unsigned */
		val = 0xFFFF - (uint16_t) val + 1;
		#if SIGNED_SIGN
			minus_sign = true;
		#endif
	}
	
	for(uint8_t i = 0; i < DIG_NUMBER; i++)
	{
		dig_val = val % 10;
		
		#if DIMM_DIGIT
			if( (val != 0) || (i < 1) || (i < dot) )
				Seg7[i] = Seg7_code[dig_val];
			else
			{
				if(minus_sign)
				{
					Seg7[i] = MINUS_SIGN;
					minus_sign = false;
				}else
					Seg7[i] = DIMM_VAL;
			}
		#else
			Seg7[i] = Seg7_code[dig_val];
		#endif
		
		val = val / 10;
		
		if( dot == i + 1)
			Seg7[i] &= DOT_BIT;

	}
}
