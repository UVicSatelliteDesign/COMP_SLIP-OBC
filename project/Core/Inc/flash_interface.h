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
A general use flash operation interface

*/
/*
memory_address: a pointer to the start address of the source data in memory
memory_size: the amount of memory to copy to flash
flash_address: the destination address in flash memory
*/

bool flash_write(void* memory_address, int memory_size, uint32_t flash_address);