#include "obc_interface.h"
#include "fatfs.h"

FRESULT res; /* FatFs function common result code */
uint32_t byteswritten, bytesread; /* File write/read counts */
uint8_t wtext[] = "STM32 FATFS works great!"; /* File write buffer */
uint8_t rtext[_MAX_SS];/* File read buffer */

// Function to store telemetry/errors/etc on SD card
void store_data(/*arguments*/){

}

// Function to store images on SD card
void store_image(/*arguments*/){

}
