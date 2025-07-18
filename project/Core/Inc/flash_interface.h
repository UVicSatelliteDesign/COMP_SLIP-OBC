#include "stm32h7xx_hal.h"
#include "fatfs.h"
#include "camera.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// Flash sector definitions
#define FLASH_SECTOR_CAMERA     1
#define FLASH_SECTOR_ALTIMETER  5
#define FLASH_SECTOR_GPS        6
#define FLASH_SECTOR_BATTERY    7

// Flash sector addresses (based on STM32H7xx flash layout)
#define FLASH_SECTOR_1_ADDRESS  0x08020000
#define FLASH_SECTOR_5_ADDRESS  0x080A0000
#define FLASH_SECTOR_6_ADDRESS  0x080C0000
#define FLASH_SECTOR_7_ADDRESS  0x080E0000

// Flash sector size (128KB for STM32H7xx)
#define FLASH_SECTOR_SIZE       0x20000

/*
flash_write: 
    writes a section of memory into a specified location in flash memory, returns success or failure
Parameters:
    memory_address: a pointer to the start address of the source data in memory
    memory_size: the amount of memory to copy to flash
    flash_sector_flag: the flash sector number (1, 5, 6, or 7)
Return:
    returns a true boolean for success and a false boolean for failure 
*/
bool flash_write(void* memory_address, int memory_size, uint8_t flash_sector_flag);

/*
flash_read:
    reads a section of flash memory to a destination address in OBC memory
Parameters:
    memory_address: a pointer to the start address of the destination in memory
    memory_size: the amount of memory to copy from flash
    flash_sector_flag: the flash sector number (1, 5, 6, or 7)
Return:
    returns a true boolean for success and a false boolean for failure   
*/
bool flash_read(void* memory_address, int memory_size, uint8_t flash_sector_flag);

/*
flash_clear:
    erases the entire flash sector, setting all bits to 1 (0xFF)
Parameters:
    flash_sector_flag: the flash sector number (1, 5, 6, or 7)
Return:
    returns a true boolean for success and a false boolean for failure   
*/
bool flash_clear(uint8_t flash_sector_flag);