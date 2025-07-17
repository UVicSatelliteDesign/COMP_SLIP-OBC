#include "flash_interface.h"
#include <string.h>

#define FLASH_MAGIC ((uint32_t)0xDEADBEEF)

FlashResult_t flash_write_data(FlashDataType_t type, const void* data, uint32_t size, uint32_t sector) {
    if (data == NULL || size == 0) {
        return FLASH_ERROR_INVALID_PARAMS;
    }

    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        return FLASH_ERROR_UNLOCK;
    }

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Sector = sector,
        .NbSectors = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };

    uint32_t pageError;
    status = HAL_FLASHEx_Erase(&erase, &pageError);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return FLASH_ERROR_ERASE;
    }

    uint32_t flashAddress;
    if (type == FLASH_TYPE_IMAGE) {
        flashAddress = FLASH_BASE + (sector * FLASH_SECTOR_SIZE);
        
        for (uint32_t i = 0; i < size / 4; i++) {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, 
                                     flashAddress + (i * 4), 
                                     ((uint32_t *)data)[i]);
            if (status != HAL_OK) {
                HAL_FLASH_Lock();
                return FLASH_ERROR_PROGRAM;
            }
        }
    } else {
        // Store both battery and sensor data in FLASH_SECTOR_7
        flashAddress = (uint32_t)0x081E0000;
        if (type == FLASH_TYPE_SENSORS) {
            flashAddress += sizeof(BatteryData); // Offset sensor data after battery data
        }
        
        uint64_t *src = (uint64_t *)data;
        uint32_t numWords = (size + 7) / 8;
        
        for (uint32_t i = 0; i < numWords; i++) {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, 
                                     flashAddress + (i * 8), 
                                     src[i]);
            if (status != HAL_OK) {
                HAL_FLASH_Lock();
                return FLASH_ERROR_PROGRAM;
            }
        }
    }

    HAL_FLASH_Lock();
    return FLASH_SUCCESS;
}

FlashResult_t flash_read_data(FlashDataType_t type, void* data, uint32_t size, uint32_t sector, uint32_t expected_magic) {
    if (data == NULL || size == 0) {
        return FLASH_ERROR_INVALID_PARAMS;
    }

    uint32_t flashAddress;
    if (type == FLASH_TYPE_IMAGE) {
        flashAddress = FLASH_BASE + (sector * FLASH_SECTOR_SIZE);
        memcpy(data, (void*)flashAddress, size);
    } else {
        // Read from FLASH_SECTOR_7 for both battery and sensor data
        flashAddress = (uint32_t)0x081E0000;
        if (type == FLASH_TYPE_SENSORS) {
            flashAddress += sizeof(BatteryData); // Offset sensor data after battery data
        }
        
        void *flash_data = (void*)flashAddress;
        uint32_t *magic_ptr = (uint32_t*)((uint8_t*)flash_data + size - sizeof(uint32_t));
        
        if (*magic_ptr != expected_magic) {
            memset(data, 0, size);
            return FLASH_ERROR_INVALID_MAGIC;
        }
        
        memcpy(data, flash_data, size);
    }

    return FLASH_SUCCESS;
}