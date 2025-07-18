#include "stm32h7xx_hal.h"
#include "fatfs.h"
#include "camera.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/*
flash_write: 
    writes a section of memory into a specified location in flash memory, returns success or failure
Parameters:
    memory_address: a pointer to the start address of the source data in memory
    memory_size: the amount of memory to copy to flash
    flash_address: the destination address in flash memory
Return:
    returns a true boolean for success and a false boolean for failure 
*/
bool flash_write(void* memory_address, int memory_size, uint16_t flash_address);

/*
flash_read:
    reads a section of flash memory to a destination address in OBC memory
Parameters:
    memory_address: a pointer to the start address of the destination in memory
    memory_size: the amount of memory to copy from flash
    flash_address: the source address in flash memory
Return:
    returns a true boolean for success and a false boolean for failure   
*/
bool flash_read(void* memory_address, int memory_size, uint16_t flash_address);