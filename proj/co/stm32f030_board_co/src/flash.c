#include "flash.h"
#include "stm32f0xx.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "spi.h"

#define FLASH_READ_STATUS_REG       (0x05u)
#define FLASH_READ_UNIQUE_ID        (0x4Bu)
#define FLASH_READ_MANUF_DEVICE_ID  (0x90u)
#define FLASH_READ_IDENTIFIC        (0x9Fu)

uint8_t flashTestBuf[3u];
void flashTest(void)
{
    spiSetCS_Low();

    spiWriteByte(FLASH_READ_IDENTIFIC);
    spiRead(&flashTestBuf[0u], 3u);

    spiSetCS_High();
}

void flashInit(void)
{
    spiInit();
    flashTest();
}