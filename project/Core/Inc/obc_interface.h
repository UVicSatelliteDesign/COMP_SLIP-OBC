#include "fatfs.h"
#include "camera.h"

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings

// For type argument of store_data
#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

// Functions for storing to SD card
FRESULT mount_SD();
FRESULT format_SD();
FRESULT setup_SD();
FRESULT store_data(uint8_t data[MAX_DATA_SIZE], uint8_t type);
FRESULT store_image(uint8_t data[MAX_IMAGE_BUFFER_SIZE]);
FRESULT unmount_SD();
