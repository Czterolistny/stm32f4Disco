#include "flash.h"
#include "stm32f0xx.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "spi.h"
#include <stdbool.h>

#define FLASH_READ_STATUS_REG       (0x05u)
#define FLASH_READ_UNIQUE_ID        (0x4Bu)
#define FLASH_READ_MANUF_DEVICE_ID  (0x90u)
#define FLASH_READ_IDENTIFIC        (0x9Fu)
#define FLASH_READ_BYTE             (0x03u)

#define FLASH_ADDR_BYTES            (0x03u)

static bool g_flashInProcess = false;

#define flashGetAreaPtr(obj,addr) ({ \
                                        do {\
                                            g_area.address = addr; \
                                            g_area.size = sizeof(obj); \
                                            g_area.object = (void*) &obj; \
                                        }while (0u); \
                                        &g_area; \
                                    })

typedef struct{
    uint32_t address;
    uint16_t size;
    void *object;
}flashArea;
static flashArea g_area;

uint8_t flashTestBuf[3u];
void flashTest(void)
{
    spiSetCS_Low();

    spiWriteByte(FLASH_READ_IDENTIFIC);
    spiRead(&flashTestBuf[0u], 3u);

    spiSetCS_High();
}

uint8_t flashReadBlock(uint32_t start_addr, uint32_t size, uint8_t *buf, uint8_t chunk)
{
    static uint32_t bytesRead;
    uint8_t interChunk = chunk;
    if( size < chunk )
    {
        interChunk = size;
    }else
    {
        if( (bytesRead + interChunk) > size)
        {
            interChunk = interChunk - ((bytesRead + (uint32_t)interChunk) - size);
        }
    }

    if( false == g_flashInProcess )
    {
        spiSetCS_Low();

        spiWriteByte(FLASH_READ_IDENTIFIC);
        flashTestBuf[0u] = (uint8_t)(start_addr & 0xFF0000u);
        flashTestBuf[1u] = (uint8_t)(start_addr & 0xFF00u);
        flashTestBuf[2u] = (uint8_t)(start_addr & 0xFFu);
        spiWrite(&flashTestBuf[0u], FLASH_ADDR_BYTES);
        g_flashInProcess = true;
        bytesRead = 0u;
    }

    spiRead(buf, interChunk);
    bytesRead += interChunk;
    if( bytesRead >= size )
    {
        g_flashInProcess = false;
        return 0;
    }
    return 1;

}

void flashReadObj(flashObj1 *obj)
{
    flashArea *area =  flashGetAreaPtr(obj, TEST_AREA_ADDR);
    flashReadBlock(area->address, area->size, (uint8_t*) (area->object), area->size);
}

void flashInit(void)
{
    spiInit();
    flashTest();
    flashReadObj(&obj1);
}