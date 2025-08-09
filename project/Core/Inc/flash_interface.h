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
#define FLASH_SECTOR_CAMERA     1U
#define FLASH_SECTOR_2          2U  // Camera overflow
#define FLASH_SECTOR_3          3U  // Camera overflow
#define FLASH_SECTOR_4          4U  // UNUSED
#define FLASH_SECTOR_ALTIMETER  5U
#define FLASH_SECTOR_GPS        6U  // Shares sector 6 with iamge data length
#define FLASH_SECTOR_IMAGE_DATA_LEN 6U // Consolidated into sector 6 with gps data
#define FLASH_SECTOR_BATTERY    7U  // Shares sector 7 with sensor data
#define FLASH_SECTOR_SENSORS    7U  // Consolidated into sector 7 with battery data

// Note: Flash sector size and base addresses are provided by STM32 HAL
// FLASH_SECTOR_SIZE (128KB) and FLASH_BANK1_BASE are defined in stm32h743xx.h

// Sector 6 memory layout offsets - GPS data and image data length
// Layout: [GPS Data][Padding][Image Data Length][Remaining Space]
#define IMAGE_DATA_LEN_OFFSET   64U     // Image data length stored after GPS data + padding

// Sector 7 memory layout offsets - Both battery and sensor data are stored in sector 7
// Layout: [Battery Data + Magic][Padding][Sensor Data + Magic][Remaining Space]
#define BATTERY_DATA_OFFSET     0U      // Battery data starts at sector beginning
#define SENSOR_DATA_OFFSET      64U     // Sensor data starts after battery data + padding

/*
flash_write: 
    writes a section of memory into a specified location in flash memory, returns success or failure
    NOTE: Sector must be cleared (erased) before writing. Call flash_clear first or ensure 
    the calling code handles sector clearing before writing.
Parameters:
    memory_address: a pointer to the start address of the source data in memory
    memory_size: the amount of memory to copy to flash
    flash_sector_flag: the flash sector number (1, 2, 3, 4, 5, 6, or 7)
    offset_address: OPTIONAL offset within the sector (0 = sector start, non-zero = specific offset)
Return:
    returns a true boolean for success and a false boolean for failure 
*/
bool flash_write(void* memory_address, int memory_size, uint8_t flash_sector_flag, uint32_t offset_address);

/*
flash_read:
    reads a section of flash memory to a destination address in OBC memory
Parameters:
    memory_address: a pointer to the start address of the destination in memory
    memory_size: the amount of memory to copy from flash
    flash_sector_flag: the flash sector number (1, 2, 3, 4, 5, 6, or 7)
    offset_address: OPTIONAL offset within the sector (0 = sector start, non-zero = specific offset)
Return:
    returns a true boolean for success and a false boolean for failure   
*/
bool flash_read(void* memory_address, int memory_size, uint8_t flash_sector_flag, uint32_t offset_address);

/*
flash_clear:
    erases the entire flash sector, setting all bits to 1 (0xFF)
    Must be called before flash_write to prepare the sector for new data.
Parameters:
    flash_sector_flag: the flash sector number (1, 2, 3, 4, 5, 6, or 7)
Return:
    returns a true boolean for success and a false boolean for failure   
*/
bool flash_clear(uint8_t flash_sector_flag);