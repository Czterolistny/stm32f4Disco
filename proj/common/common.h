#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

uint8_t uint_to_string(char *s, uint16_t x);
uint8_t _strlen(char *s);
uint16_t crc16calc(const char *buf, uint16_t crc, uint16_t len, uint16_t poly);

#endif