#include <stdint.h>
#include "common.h"

uint8_t uint_to_string(char *s, uint16_t x)
{
	uint8_t i = 0;
	if(x == 0){
		*s++ = '0';
		*s++ = ' ';
		i += 2;
		return i;
	}
	
	uint16_t div = 10000;
	while( div ){
		if( x/div ){
			*s++ = ((x/div) - (x / (div*10)) * 10) + '0';
			i++;
		}
		div = div / 10;
	}
	*s = ' ';
	i++;
	return i;
}

uint8_t _strlen(char *s)
{
	uint8_t len = 0;
	while( *s++ != 0 )
		len++;
	return len;
}