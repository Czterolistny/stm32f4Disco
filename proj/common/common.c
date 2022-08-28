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

uint16_t crc16calc(const char *buf, uint16_t crc, uint16_t len, uint16_t poly)
{
	uint16_t i, tmp_crc = crc;

	while (len--)
    {
		tmp_crc = *buf++ << 8;
	    crc = tmp_crc ^ crc;

	    for(i = 0; i < 8; ++i)
        {
		    if(crc > 32767)
			 	crc = poly ^ (crc << 1);
            else
			    crc = crc << 1;
			
			crc = crc & 0xFFFF;
        }
    }

    return crc;
}