#ifndef OBC_INTERFACE_H
#define OBC_INTERFACE_H

#include "main.h"
#include "fatfs.h"

#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings
#define MAX_IMAGE_BUFFER_SIZE 100 // TODO: Temp

// Battery data structure
typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float power_usage;
    float total_energy_used;
    float estimated_life;
    uint32_t magic;
} BatteryData;

typedef struct {
	float temperature;        	// temperature, celsius
	float pressure;             	// pressure, unit tbd
	float gyroscope_axis_1;     // gyroscope x axis
	float gyroscope_axis_2;     // gyroscope y axis
	float gyroscope_axis_3;     // gyroscope z axis
	float acceleration_axis_1;  // accelerometer x axis
	float acceleration_axis_2;  // accelerometer y axis
	float acceleration_axis_3;  // accelerometer z axis
	uint32_t magic;
} SensorsData;

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

// Dummy functions
void freeImageBuffer();
int capture_snapshot();
void save_image_to_flash();
void Flash_Read_Data();
void save_battery_data_to_flash(BatteryData *data);
BatteryData load_battery_data_from_flash();
void save_sensor_data_to_flash(SensorsData *data);
SensorsData load_sensor_data_from_flash();
SensorsData read_sensors();
float read_gyroscope_x1();
float read_gyroscope_x2();
float read_gyroscope_x3();
float read_acceleration_x1();
float read_acceleration_x2();
float read_acceleration_x3();

#endif // OBC_INTERFACE_H
