#ifndef __SWUART_H__
#define __SWUART_H__

void swuartInit(void);
void swuartSend(uint8_t *buf, uint8_t len);

typedef void (*swuartRxCompleteCallb)(uint8_t, uint16_t);
typedef void (*swuartTxCompleteCallb) (uint16_t);
void swuartInitClb(swuartTxCompleteCallb txClb, swuartRxCompleteCallb rxClb);

#endif