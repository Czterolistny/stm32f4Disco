#ifndef __SWUART_H__
#define __SWUART_H__

typedef struct
{
    void (*swuartRxCompleteClb) (uint8_t);
    void (*swuartRxOneByteCompleteClb) (uint8_t, uint8_t);
    void (*swuartTxCompleteClb) (uint16_t);
}swUartConfigType;
extern swUartConfigType swUartConfig;

void swuartInit(swUartConfigType *config);
void swuartSend(uint8_t *buf, uint8_t len);

#endif