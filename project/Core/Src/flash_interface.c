#include "flash_interface.h"
#include "stm32h7xx_hal.h"
/*
this will contain flash storage and retrieval functions
functions are documented in flash_interface.h
*/

bool flash_write(void* memory_address, int memory_size, uint16_t flash_address);

bool flash_read(void* memory_address, int memory_size, uint16_t flash_address);