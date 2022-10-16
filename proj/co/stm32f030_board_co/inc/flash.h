#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#define __PACKED__ __attribute__((aligned(0)))

#define TEST_AREA_ADDR  ((uint32_t)0x00u)

typedef struct{
    uint8_t param1;
    uint16_t param2;
}__PACKED__ flashObj1;
flashObj1 obj1;

void flashInit(void);
void flashReadObj(flashObj1 *obj);
void flashWriteObj(flashObj1 *obj);
uint8_t flashReadBlock(uint32_t start_addr, uint32_t size, uint8_t *buf, uint8_t chunk);

#endif