#include "flash_interface.h"
#include "stm32h7xx_hal.h"
/*
this will contain flash storage and retrieval functions
functions are documented in flash_interface.h
*/

static uint32_t get_sector_address(uint8_t flash_sector_flag) {
    // Validate sector range (sectors 1-7 are used, sector 0 is reserved)
    if (flash_sector_flag < 1U || flash_sector_flag > 7U) {
        return 0;
    }
    
    // Calculate address using HAL constants: FLASH_BANK1_BASE + (sector * FLASH_SECTOR_SIZE)
    // This gives us the correct offset addressing that starts from sector 1
    return FLASH_BANK1_BASE + (flash_sector_flag * FLASH_SECTOR_SIZE);
}


bool flash_read(void* memory_address, int memory_size, uint8_t flash_sector_flag, uint32_t offset_address) {
    if (memory_address == NULL || memory_size <= 0) {
        return false;
    }
    
    uint32_t sector_address = get_sector_address(flash_sector_flag);
    if (sector_address == 0) {
        return false;
    }
    
    uint32_t read_address = sector_address + offset_address;
    
    // Validate that read doesn't exceed sector boundary
    if (offset_address + memory_size + 4 > FLASH_SECTOR_SIZE) {
        return false;
    }
    
    uint8_t *flash_data = (uint8_t *)read_address;
    uint8_t *dest = (uint8_t *)memory_address;
    
    if (*(uint32_t*)flash_data == 0xDEADBEEF) {
        memcpy(dest, flash_data + 4, memory_size);
        return true;
    } else {
        memset(dest, 0, memory_size);
        return false;
    }
}

bool flash_write(void* memory_address, int memory_size, uint8_t flash_sector_flag, uint32_t offset_address) {
    if (memory_address == NULL || memory_size <= 0) {
        return false;
    }
    
    uint32_t sector_address = get_sector_address(flash_sector_flag);
    
    if (sector_address == 0) {
        return false;
    }
    
    uint32_t write_address = sector_address + offset_address;
    
    // Validate that write doesn't exceed sector boundary
    if (offset_address + memory_size + 4 > FLASH_SECTOR_SIZE) {
        return false;
    }
    
    HAL_FLASH_Unlock();
    
    // Write magic number first
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_address, 0xDEADBEEF) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    uint64_t *src = (uint64_t *)memory_address;
    uint32_t numWords = (memory_size + 7) / 8;
    
    for (uint32_t i = 0; i < numWords; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, write_address + 4 + (i * 8), src[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }
    
    HAL_FLASH_Lock();
    return true;
}

bool flash_clear(uint8_t flash_sector_flag) {
    if (flash_sector_flag < FLASH_SECTOR_CAMERA || flash_sector_flag > FLASH_SECTOR_BATTERY) {
        return false;
    }
    
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;
    
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = flash_sector_flag;
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    HAL_FLASH_Lock();
    return true;
}
