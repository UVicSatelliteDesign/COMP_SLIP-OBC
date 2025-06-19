#ifndef SRC_OBC_INTERFACE_H
#define SRC_OBC_INTERFACE_H

#include "main.h"
#include "fatfs.h"

// For type argument of store_data
#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings
#define MAX_IMAGE_BUFFER_SIZE 100 // TODO: Change to actual value

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

// BMS prototypes
void init_bms();
float read_battery_voltage();
float read_battery_current();
float read_battery_temperature();
float calculate_state_of_charge(float current, float dt);
float calculate_state_of_charge(float current, float dt);
float calculate_power_usage(float voltage, float current);
float calculate_energy_usage(float power, float dt);
float estimate_battery_life(float state_of_charge, float avg_power_draw);
BatteryData get_battery_data(float dt);
void save_battery_data_to_flash(BatteryData *data);
void load_battery_data_from_flash();

////////////sensors start
typedef struct { 		// struct containing sensor data values
    float temperature;        	// temperature, celsius
    float pressure;             	// pressure, unit tbd
    float gyroscope_axis_1;     // gyroscope x axis
    float gyroscope_axis_2;     // gyroscope y axis
    float gyroscope_axis_3;     // gyroscope z axis
    float acceleration_axis_1;  // accelerometer x axis
    float acceleration_axis_2;  // accelerometer y axis
    float acceleration_axis_3;  // accelerometer z axis
    uint32_t magic;             // flash data validation value
} SensorsData;

void init_sensors(); // initialises pins
void save_sensor_data_to_flash(SensorsData *data); // handles saving to flash
void load_sensor_data_from_flash(); // retrieves flash data and uses a pointer to write to the sensor_backup struct
SensorsData read_sensors(); // polls sensors, calls save_sensor_data_to_flash, returns sensor data object

float read_temperature(); // poll temperature sensor
float read_pressure();	// poll pressure sensor

float read_gyroscope_x1(); // poll gyroscope
float read_gyroscope_x2();
float read_gyroscope_x3();

float read_acceleration_x1(); // poll accelerometer
float read_acceleration_x2();
float read_acceleration_x3();
/////////////////sensors end

// Memory prototypes
FRESULT mount_SD();
FRESULT format_SD();
FRESULT setup_SD();
FRESULT store_data(uint8_t data[MAX_DATA_SIZE], uint8_t type);
FRESULT store_image(uint8_t data[MAX_IMAGE_BUFFER_SIZE]);
FRESULT unmount_SD();

#endif /* SRC_OBC_INTERFACE_H_ */
