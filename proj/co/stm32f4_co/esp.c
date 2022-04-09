#include <stdio.h>
#if defined STM32F4XX
	#include "stm32f4xx_usart.h"
#elif defined STM32F0XX
	#include "stm32f0xx_usart.h"
#endif
#include <stdint.h>
#include "main.h"
#include "esp.h"

#define CRC_INIT 0xB169

extern const uint8_t param[];
const uint8_t estabUDPInitcmd[] = {'A', 'T', '+', 'C', 'I', 'P', 'S', 'T', 'A', 'R', 'T', '=', '"', 'U', 'D', 'P', \
                                    '"', ',', '"', '1', '9', '2', '.', '1', '6', '8', '.', '1', '.', '1', '0', '5', \
                                    '"', ',', '8', '9', '0', '1', ',', '8', '0', '0', '1', '\r', '\n'};

static uint16_t crc16calc(char *buf, uint16_t crc, uint16_t len);
static uint8_t uint_to_string(char *s, uint16_t x);
static uint8_t _strlen(char *s);
inline static void sendATcmd(char *cmd, uint8_t len);

espConfig *config;

void espInit(espConfig *conf)
{
	if( (NULL != conf) && (NULL != conf->espWriteATCommand) )
	{
		config = conf;
	}else
		return;

	config->espWriteATCommand((uint8_t *) &estabUDPInitcmd[0], sizeof(estabUDPInitcmd)/sizeof(estabUDPInitcmd[0]));

	delay_ms(1);
}

static void sendATcmd(char *cmd, uint8_t len)
{
	config->espWriteATCommand((uint8_t*) cmd, len);
}

void sendToESP(uint8_t *recv_buf, uint8_t recv_len)
{
	char udp_buf[32];
	
	uint8_t buf_len = uint_to_string(&udp_buf[0], recv_buf[param[SET_TEMP]]);
	buf_len += uint_to_string(&udp_buf[buf_len], ((recv_buf[param[TEMP1]] << 8) | recv_buf[param[TEMP1] +1]) / 10);
	buf_len += uint_to_string(&udp_buf[buf_len], ((recv_buf[param[TEMP2]] << 8) | recv_buf[param[TEMP2] +1]) / 10);
	buf_len += uint_to_string(&udp_buf[buf_len], ((recv_buf[param[EXH_TEMP]] << 8) | recv_buf[param[EXH_TEMP] +1]) / 10);
	buf_len += uint_to_string(&udp_buf[buf_len], recv_buf[param[FAN_PERC]]);
	udp_buf[buf_len++] = '\n';
	
	char space = ' ';
	uint16_t crc16 = crc16calc(&space, CRC_INIT, 1);
	crc16 = crc16calc(&udp_buf[0], crc16, buf_len);
	char crc16_str[5] = "0";
	uint8_t crc16_len = uint_to_string(&crc16_str[0], crc16);
	
	char at_cmd[18] = "AT+CIPSEND=";
	uint8_t at_len = _strlen(at_cmd);
	at_len += uint_to_string(&at_cmd[at_len], buf_len + crc16_len);
	at_cmd[at_len -1] = '\r';
	at_cmd[at_len++] = '\n';
	
	sendATcmd(&at_cmd[0], at_len);
	delay_ms(1);
	sendATcmd(&crc16_str[0], crc16_len);
	sendATcmd(&udp_buf[0], buf_len);
	
}

static uint16_t crc16calc(char *buf, uint16_t crc, uint16_t len)
{
	uint16_t i, tmp_crc = crc, poly=0x2f5c;

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

static uint8_t uint_to_string(char *s, uint16_t x)
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

static uint8_t _strlen(char *s)
{
	uint8_t len = 0;
	while( *s++ != 0 )
		len++;
	return len;
}