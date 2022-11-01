#include "flash.h"
#include "stm32f0xx.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "spi.h"
#include <stdbool.h>

#define FLASH_READ_STATUS_REG_CMD       (0x05u)
#define FLASH_WRITE_STATUS_REG_CMD      (0x01u)
#define FLASH_READ_UNIQUE_ID_CMD        (0x4Bu)
#define FLASH_READ_MANUF_DEVICE_ID_CMD  (0x90u)
#define FLASH_READ_IDENTIFIC_CMD        (0x9Fu)

#define FLASH_WR_ENABLE_CMD             (0x06u)
#define FLASH_WR_DISABLE_CMD            (0x04u)
#define FLASH_WRITE_STAT_REG_CMD        (0x01u)
#define FLASH_READ_DATA_CMD             (0x03u)
#define FLASH_WRITE_DATA_CMD            (0x02u)

#define FLASH_WIP_BIT_IDX               (0x01u << 0u)

#define FLASH_ADDR_BYTES                (0x03u)

#define flashGetAddrBuf(addr_24) ({ \
                                    do {\
                                        g_flashBuf[0u] = (uint8_t)(start_addr & 0xFF0000u); \
                                        g_flashBuf[1u] = (uint8_t)(start_addr & 0xFF00u); \
                                        g_flashBuf[2u] = (uint8_t)(start_addr & 0xFFu); \
                                    }while (0u); \
                                    &g_flashBuf[0u]; \
                                })

#define flashGetCmdBufRef()     (&g_flashBuf[0u])

static bool g_flashBlockOperationInProgress = false;
static uint8_t g_flashBuf[4u];

static flashObj1 obj1 = {.param1 = 0xA5u, .param2 = 0xBCDF};
flashAreaObj areaObj1 = {.addr = TEST_OBJ_ADDR, .size = TEST_OBJ_SIZE, .obj = (void*)&obj1};

typedef struct{
    uint8_t cmd_id;
    uint8_t byteToWrite;
    int8_t byteToRead;
    uint8_t *cmdBuf;
}FlashCmd;

typedef enum{
    FLASH_READ_IDENTIFIC_CMD_IDX = 0,
    FLASH_WR_ENABLE_CMD_IDX,
    FLASH_WR_DISABLE_CMD_IDX,
    FLASH_READ_STATUS_REG_CMD_IDX,
    FLASH_WRITE_STATUS_REG_CMD_IDX,
    FLASH_READ_DATA_CMD_IDX
}CmdType;

static FlashCmd flashCmd[] = {
    {.cmd_id = FLASH_READ_IDENTIFIC_CMD, .byteToWrite = 0u, .byteToRead = 3, .cmdBuf = &g_flashBuf[0u]},
    {.cmd_id = FLASH_WR_ENABLE_CMD, .byteToWrite = 0u, .byteToRead = 0, .cmdBuf = &g_flashBuf[0u]},
    {.cmd_id = FLASH_WR_DISABLE_CMD, .byteToWrite = 0u, .byteToRead = 0, .cmdBuf = &g_flashBuf[0u]},
    {.cmd_id = FLASH_READ_STATUS_REG_CMD, .byteToWrite = 0u, .byteToRead = 1, .cmdBuf = &g_flashBuf[0u]},
    {.cmd_id = FLASH_WRITE_STATUS_REG_CMD, .byteToWrite = 1u, .byteToRead = 0, .cmdBuf = &g_flashBuf[0u]},
    {.cmd_id = FLASH_READ_DATA_CMD, .byteToWrite = 3u, .byteToRead = -1, .cmdBuf = &g_flashBuf[0u]}};

static void flashWriteCmd(CmdType cmd_idx)
{
    spiSetCS_Low();
    spiWriteByte(flashCmd[cmd_idx].cmd_id);
    spiWrite(flashCmd[cmd_idx].cmdBuf, flashCmd[cmd_idx].byteToWrite);
    /* Do not finish transmission if command requires unexpected amount of byte to read */
    if( flashCmd[cmd_idx].byteToRead >= 0 )
    {
        spiRead(flashCmd[cmd_idx].cmdBuf, flashCmd[cmd_idx].byteToRead);
        spiSetCS_High();
    }else{
        /* CS has to be set 'by hand' after trasmition get completed */
    }
}

inline static bool flashIsWriteInProgress(void)
{
    flashWriteCmd(FLASH_READ_STATUS_REG_CMD_IDX);
    return (bool) (FLASH_WIP_BIT_IDX & (*flashGetCmdBufRef()));
}

static uint8_t flashGetInterChunkSize(uint32_t size, uint8_t chunk)
{
    static uint32_t bytesRead;
    uint8_t interChunk = chunk;
    if( size < chunk )
    {
        interChunk = size;
    }else
    {
        if( (bytesRead + interChunk ) > size)
        {
            interChunk = interChunk - (uint8_t)((bytesRead + (uint32_t)interChunk) - size);
        }
    }

    bytesRead += interChunk;
    if( bytesRead >= size )
    {
        bytesRead = 0u;
    }

    return interChunk;
}

void flashTest(void)
{
    flashWriteCmd(FLASH_READ_IDENTIFIC_CMD_IDX);
    uint8_t *res = flashGetCmdBufRef();
}

uint8_t flashWriteBlock(uint32_t start_addr, uint32_t size, uint8_t *buf, uint8_t chunk)
{
    uint8_t interChunk;
    if( false == g_flashBlockOperationInProgress )
    {
        flashWriteCmd(FLASH_WR_ENABLE_CMD_IDX);

        while( true == flashIsWriteInProgress() ){};
        spiSetCS_Low();

        spiWriteByte(FLASH_WRITE_DATA_CMD);
        uint8_t *addr = flashGetAddrBuf(start_addr);
        spiWrite(addr, FLASH_ADDR_BYTES);
        g_flashBlockOperationInProgress = true;
    }

    interChunk = flashGetInterChunkSize(size, chunk);
    spiRead(buf, interChunk);
    if( interChunk < chunk )
    {
        g_flashBlockOperationInProgress = false;
        spiSetCS_High();
        flashWriteCmd(FLASH_WR_DISABLE_CMD_IDX);
        return 0;
    }
    return 1;
}

uint8_t flashReadBlock(uint32_t start_addr, uint32_t size, uint8_t *buf, uint8_t chunk)
{
    uint8_t interChunk;
    if( false == g_flashBlockOperationInProgress )
    {
        spiSetCS_Low();

        spiWriteByte(FLASH_WRITE_DATA_CMD);
        uint8_t *addr = flashGetAddrBuf(start_addr);
        spiWrite(addr, FLASH_ADDR_BYTES);
        g_flashBlockOperationInProgress = true;
    }

    interChunk = flashGetInterChunkSize(size, chunk);
    spiRead(buf, interChunk);
    if( interChunk < chunk )
    {
        g_flashBlockOperationInProgress = false;
        spiSetCS_High();
        return 0;
    }
    return 1;
}

void flashWriteAreaObj(flashAreaObj *areaObj)
{
    flashWriteBlock(areaObj->addr, areaObj->size, (uint8_t*) (&areaObj->obj), areaObj->size);
}

void flashReadAreaObj(flashAreaObj *areaObj)
{
    flashReadBlock(areaObj->addr, areaObj->size, (uint8_t*) (&areaObj->obj), areaObj->size);
}

void flashInit(void)
{
    spiInit();
    flashTest();
    flashReadAreaObj(&areaObj1);
}