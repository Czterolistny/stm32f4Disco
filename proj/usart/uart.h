#ifndef _UART_H_
#define _UART_H_

#define TX_BUF_SIZE 32
#define RX_BUF_SIZE 32
#define DMA_BUF_SIZE 16

void USART2_Init(void);
uint8_t USART2_SendNoneBlocking(uint8_t *bytes, uint16_t len);
void USART2_SendBlocking(uint8_t *bytes, uint16_t len);
void USART2_SendString(char *str);
void USART2_Send_Byte(uint8_t byte);

uint8_t USART2_DMA_Send(uint8_t *bytes, uint16_t len);
uint8_t registerRxCompleteCallb( void (*callback)(uint8_t*, uint16_t));
uint8_t registerTxCompleteCallb( void (*callback)(uint16_t));
	
#endif
