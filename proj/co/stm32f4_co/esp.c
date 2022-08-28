#include <stdio.h>
#if defined STM32F4XX
	#include "stm32f4xx_usart.h"
#elif defined STM32F0XX
	#include "stm32f0xx_usart.h"
#endif
#include <stdint.h>
#include "esp.h"
#include "../../common/common.h"

#define ESP_CRC_INIT 	(0xB169)
#define ESP_CRC_POLY 	(0x2f5c)
const char startSign = ' ';

extern const uint8_t param[];
const uint8_t estabUDPInitcmd[] = {'A', 'T', '+', 'C', 'I', 'P', 'S', 'T', 'A', 'R', 'T', '=', '"', 'U', 'D', 'P', \
                                    '"', ',', '"', '1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1', '0', '5', \
                                    '"', ',', '8', '9', '0', '1', ',', '8', '0', '0', '1', '\r', '\n'};

inline static void espSendATcmd(char *cmd, uint8_t len);

const espConfig *config;

void espInit(espConfig *conf)
{
	if( (NULL != conf) && (NULL != conf->espWriteATCommand) )
	{
		config = conf;
	}else
		return;

	(*config->espWriteATCommand)((uint8_t *) &estabUDPInitcmd[0], sizeof(estabUDPInitcmd)/sizeof(estabUDPInitcmd[0]));

	delay_ms(1);
}

static void espSendATcmd(char *cmd, uint8_t len)
{
	(*config->espWriteATCommand)((uint8_t*) cmd, len);
}

static uint16_t espGetCRC(char *buf, uint8_t len)
{
	uint16_t crc16 = crc16calc(&startSign, ESP_CRC_INIT, 1, ESP_CRC_POLY);
	return crc16calc(&buf[0], crc16, len, ESP_CRC_POLY);
}

void espWrite(CoParamType *p)
{
	char udp_buf[32];
	char crc16_str[5] = {0};
	char at_cmd[18] = "AT+CIPSEND=";

	uint8_t udp_buf_len = 0u;

	for(uint8_t paramCnt = 0; paramCnt < p->paramNmb; ++paramCnt)
	{
		udp_buf_len += uint_to_string(&udp_buf[udp_buf_len], p->param[paramCnt]);
	}
	udp_buf[udp_buf_len++] = '\n';
	
	uint16_t crc16 = espGetCRC(&udp_buf[0], udp_buf_len);
	uint8_t crc16_len = uint_to_string(&crc16_str[0], crc16);
	uint8_t at_len = _strlen(at_cmd);

	at_len += uint_to_string(&at_cmd[at_len], udp_buf_len + crc16_len);

	at_cmd[at_len -1] = '\r';
	at_cmd[at_len++] = '\n';
	
	espSendATcmd(&at_cmd[0], at_len);
	delay_ms(5);
	/* TODO - Check for response - wait for comamnd prompt */
	espSendATcmd(&crc16_str[0], crc16_len);
	espSendATcmd(&udp_buf[0], udp_buf_len);
	
}