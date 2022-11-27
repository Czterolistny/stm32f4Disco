#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#define __PACKED__ __attribute__((__packed__))

#define TEST_OBJ_ADDR  ((uint32_t)0x00u)
#define TEST_OBJ_SIZE  ((uint16_t)0x03u)

#define flashGetAreaObjDataPtr(flashAreaObj) (flashAreaObj->obj)

typedef struct{
    uint8_t param1;
    uint16_t param2;
}__PACKED__ flashObj1;

typedef struct{
    const uint32_t addr;
    const uint16_t size;
    const void *obj;
}flashAreaObj;

extern flashAreaObj areaObj1;

void flashInit(void);
void flashReadAreaObj(flashAreaObj *obj);
void flashWriteAreaObj(flashAreaObj *obj);
uint8_t flashReadBlock(uint32_t start_addr, uint32_t size, uint8_t *buf, uint8_t chunk);
uint8_t flashWriteBlock(uint32_t start_addr, uint32_t size, uint8_t *buf, uint8_t chunk);
void flashEraseSector(uint16_t sector);

#endif