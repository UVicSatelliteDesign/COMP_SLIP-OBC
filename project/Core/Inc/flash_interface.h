#ifndef FLASH_INTERFACE_H
#define FLASH_INTERFACE_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

typedef enum {
    FLASH_SUCCESS = 0,
    FLASH_ERROR_UNLOCK,
    FLASH_ERROR_ERASE,
    FLASH_ERROR_PROGRAM,
    FLASH_ERROR_INVALID_PARAMS,
    FLASH_ERROR_INVALID_MAGIC
} FlashResult_t;

typedef enum {
    FLASH_TYPE_BATTERY = 0,
    FLASH_TYPE_SENSORS,
    FLASH_TYPE_IMAGE
} FlashDataType_t;

FlashResult_t flash_write_data(FlashDataType_t type, const void* data, uint32_t size, uint32_t sector);
FlashResult_t flash_read_data(FlashDataType_t type, void* data, uint32_t size, uint32_t sector, uint32_t expected_magic);
FlashResult_t flash_clear_sector(uint32_t sector);

#endif /* FLASH_INTERFACE_H */
