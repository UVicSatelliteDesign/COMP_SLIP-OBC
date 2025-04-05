#include "fatfs.h"

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings

// Functions for storing to SD card
FRESULT mount_SD();
FRESULT format_SD();
FRESULT setup_SD();
FRESULT store_data(uint8_t data[MAX_DATA_SIZE], uint8_t type);
FRESULT store_image(uint8_t data[IMAGE_BUFFER_SIZE]);
FRESULT unmount_SD();
