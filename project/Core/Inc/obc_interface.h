#ifndef SRC_OBC_INTERFACE_H_
#define SRC_OBC_INTERFACE_H_

#include "stm32h7xx_hal.h"
#include "fatfs.h"
#include "camera.h"

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings

// For type argument of store_data
#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

// Battery data structure
typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float power_usage;
    float total_energy_used;
    float estimated_life;
} BatteryData;

// Function prototypes
void init_bms(void);
BatteryData get_battery_data(float dt);

// Functions for storing to SD card
FRESULT mount_SD();
FRESULT format_SD();
FRESULT setup_SD();
FRESULT store_data(uint8_t data[MAX_DATA_SIZE], uint8_t type);
FRESULT store_image(uint8_t data[MAX_IMAGE_BUFFER_SIZE]);
FRESULT unmount_SD();

#endif /* SRC_OBC_INTERFACE_H_ */
