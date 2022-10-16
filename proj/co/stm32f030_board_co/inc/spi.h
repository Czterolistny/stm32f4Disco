#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

void spiInit(void);
void spiWriteByte(uint8_t byte);
void spiWrite(uint8_t *buf, uint8_t len);
void spiRead(uint8_t *buf, uint8_t len);

void spiSetCS_High(void);
void spiSetCS_Low(void);

#endif