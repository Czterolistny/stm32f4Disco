#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

void i2cInit(void);
void i2cWrite(uint8_t *buf, uint8_t len);
void i2cRead(uint8_t *buf, uint8_t reg_addr_bytes, uint8_t len);

#endif