#include "flash_interface.h"
#include "stm32h7xx_hal.h"
/*
this will contain flash storage and retrieval functions
functions are documented in flash_interface.h
*/

static uint32_t get_sector_address(uint8_t flash_sector_flag) {
    switch (flash_sector_flag) {
        case FLASH_SECTOR_CAMERA:
            return FLASH_SECTOR_1_ADDRESS;
        case FLASH_SECTOR_2:
            return FLASH_SECTOR_2_ADDRESS;
        case FLASH_SECTOR_3:
            return FLASH_SECTOR_3_ADDRESS;
        case FLASH_SECTOR_SENSORS:
            return FLASH_SECTOR_4_ADDRESS;
        case FLASH_SECTOR_ALTIMETER:
            return FLASH_SECTOR_5_ADDRESS;
        case FLASH_SECTOR_GPS:
            return FLASH_SECTOR_6_ADDRESS;
        case FLASH_SECTOR_BATTERY:
            return FLASH_SECTOR_7_ADDRESS;
        default:
            return 0;
    }
}

static uint32_t get_hal_sector(uint8_t flash_sector_flag) {
    switch (flash_sector_flag) {
        case FLASH_SECTOR_CAMERA:
            return FLASH_SECTOR_1;
        case FLASH_SECTOR_2:
            return FLASH_SECTOR_2;
        case FLASH_SECTOR_3:
            return FLASH_SECTOR_3;
        case FLASH_SECTOR_SENSORS:
            return FLASH_SECTOR_4;
        case FLASH_SECTOR_ALTIMETER:
            return FLASH_SECTOR_5;
        case FLASH_SECTOR_GPS:
            return FLASH_SECTOR_6;
        case FLASH_SECTOR_BATTERY:
            return FLASH_SECTOR_7;
        default:
            return 0xFFFFFFFF;
    }
}

bool flash_read(void* memory_address, int memory_size, uint8_t flash_sector_flag) {
    if (memory_address == NULL || memory_size <= 0) {
        return false;
    }
    
    uint32_t sector_address = get_sector_address(flash_sector_flag);
    if (sector_address == 0) {
        return false;
    }
    
    uint8_t *flash_data = (uint8_t *)sector_address;
    uint8_t *dest = (uint8_t *)memory_address;
    
    if (*(uint32_t*)flash_data == 0xDEADBEEF) {
        memcpy(dest, flash_data + 4, memory_size);
        return true;
    } else {
        memset(dest, 0, memory_size);
        return false;
    }
}

bool flash_write(void* memory_address, int memory_size, uint8_t flash_sector_flag) {
    if (memory_address == NULL || memory_size <= 0) {
        return false;
    }
    
    uint32_t sector_address = get_sector_address(flash_sector_flag);
    uint32_t hal_sector = get_hal_sector(flash_sector_flag);
    
    if (sector_address == 0 || hal_sector == 0xFFFFFFFF) {
        return false;
    }
    
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;
    
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = hal_sector;
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    // Write magic number first
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, sector_address, 0xDEADBEEF) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    uint64_t *src = (uint64_t *)memory_address;
    uint32_t numWords = (memory_size + 7) / 8;
    
    for (uint32_t i = 0; i < numWords; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, sector_address + 4 + (i * 8), src[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }
    
    HAL_FLASH_Lock();
    return true;
}

bool flash_clear(uint8_t flash_sector_flag) {
    uint32_t hal_sector = get_hal_sector(flash_sector_flag);
    
    if (hal_sector == 0xFFFFFFFF) {
        return false;
    }
    
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;
    
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = hal_sector;
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    HAL_FLASH_Lock();
    return true;
}