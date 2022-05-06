#ifndef __SREGS_H__
#define __SREGS_H__

#include <stdbool.h>

/* Sregs default output states - set after sregsInit() */
#define SREGS_ESP_UART_DEFAULT_STATE        ((bool) 1u)
#define SREGS_ESP_DTR_RTS_EN_DEFAULT_STATE  ((bool) 0u)
#define SREGS_UART_BOOT_EN_DEFAULT_STATE    ((bool) 0u)
#define SREGS_ESP_SW_UART_EN_DEFAULT_STATE  ((bool) 0u)
#define SREGS_ESP_RTS_DEFAULT_STATE         ((bool) 0u)
#define SREGS_ESP_DTR_DEFAULT_STATE         ((bool) 0u)
#define SREGS_SD_CS_DEFAULT_STATE           ((bool) 0u)
#define SREGS_SREG1_OUT_DEFAULT_STATE       ((bool) 1u)
#define SREGS_ESP_EN_DEFAULT_STATE          ((bool) 0u)
#define SREGS_ESP_WAKE_DEFAULT_STATE        ((bool) 1u)
#define SREGS_DIG_OUT2_DEFAULT_STATE        ((bool) 1u)
#define SREGS_MCU_RST_DEFAULT_STATE         ((bool) 0u)
#define SREGS_FLASH_CS_DEFAULT_STATE        ((bool) 0u)
#define SREGS_BOOT0_DEFAULT_STATE           ((bool) 0u)
#define SREGS_UNDEF1_DEFAULT_STATE          ((bool) 0u)
#define SREGS_UNDEF2_DEFAULT_STATE          ((bool) 1u)

/* SREG outputNmb defines */
#define SREGS_ESP_UART          (1 << 15u)
#define SREGS_ESP_DTR_RTS_EN    (1 << 14u)
#define SREGS_UART_BOOT_EN      (1 << 13u)
#define SREGS_ESP_SW_UART_EN    (1 << 12u)
#define SREGS_ESP_RTS           (1 << 11u)
#define SREGS_ESP_DTR           (1 << 10u)
#define SREGS_SD_CS             (1 << 9u)
#define SREGS_SREG1             (1 << 8u)
#define SREGS_ESP_EN            (1 << 7u)
#define SREGS_ESP_WAKE          (1 << 6u)
#define SREGS_DIG_OUT2          (1 << 5u)
#define SREGS_MCU_RST           (1 << 4u)
#define SREGS_FLASH_CS          (1 << 3u)
#define SREGS_BOOT0             (1 << 2u)
#define SREGS_UNDEF1            (1 << 1u)
#define SREGS_UNDEF2            (1 << 0u)

/* Initialize HW for Sregs and set to default state */
void sregsInit(void);
/* Set Sregs output to 'state' */
void sregsSetOutput(uint8_t outputNmb, bool state);
/* Get Sregs output */
void sregsGetOutput(uint8_t outputNmb, bool *state);

#endif