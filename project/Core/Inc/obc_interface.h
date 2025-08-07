#ifndef SRC_OBC_INTERFACE_H
#define SRC_OBC_INTERFACE_H

//#include "main.h"
#include "fatfs.h"
#include "camera.h"

// For type argument of store_data
#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings

// Battery data structure
typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float power_usage;
    float estimated_life;
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
    float temperature_obc;      // temperature, celsius
    float temperature_ttc;
    float temperature_bms;
    float gyroscope_axis_1;     // gyroscope x axis
    float gyroscope_axis_2;     // gyroscope y axis
    float gyroscope_axis_3;     // gyroscope z axis
    float acceleration_axis_1;  // accelerometer x axis
    float acceleration_axis_2;  // accelerometer y axis
    float acceleration_axis_3;  // accelerometer z axis
    float altitude;				// altimeter altitude
} SensorsData;

void init_sensors(); // initialises pins
void save_sensor_data_to_flash(SensorsData *data); // flash storage macro
void load_sensor_data_from_flash(); // flash retrieval macro

/*
read_sensors:
    macro polling sensor data to a SensorsData struct
Parameters:
    null
Return:
    returns a SensorsData struct  
*/
SensorsData read_sensors(); 

void init_bms();
float read_battery_voltage();
float read_battery_current();
float read_battery_temperature();
float calculate_state_of_charge(float current, float dt);
float calculate_power_usage(float voltage, float current);
float estimate_battery_life(float state_of_charge, float avg_power_draw);
BatteryData get_battery_data(float dt);
void save_battery_data_to_flash(BatteryData *data);
void load_battery_data_from_flash();

float read_OBC_temperature(); // poll temperature sensor
float read_TTC_temperature_1();
float read_TTC_temperature_2();
float read_BMS_temperature_1();
float read_BMS_temperature_2();

float read_gyroscope_x1(); // poll gyroscope
float read_gyroscope_x2();
float read_gyroscope_x3();

float read_acceleration_x1(); // poll accelerometer
float read_acceleration_x2();
float read_acceleration_x3();

//// Altimeter
// For use by HL
uint8_t altimeter_init(); // Returns 0 if success, 1 if not
float altimeter_read();
uint8_t altimeter_reset(); // Only if necessary, returns 0 if success, 1 if not
// For use by LL
uint32_t altimeter_read_pressure();
uint32_t altimeter_read_temperature();
uint8_t altimeter_read_calibration(); // Returns 0 if success, 1 if not
/////////////////sensors end

//// Altimeter
// For use by HL
uint8_t altimeter_init(); // Returns 0 if success, 1 if not
float altimeter_read();
uint8_t altimeter_reset(); // Only if necessary, returns 0 if success, 1 if not
// For use by LL
uint32_t altimeter_read_pressure();
uint32_t altimeter_read_temperature();
uint8_t altimeter_read_calibration(); // Returns 0 if success, 1 if not
/////////////////sensors end

// Memory prototypes
FRESULT mount_SD();
FRESULT format_SD();
FRESULT setup_SD();
FRESULT store_data(uint8_t* data, uint8_t data_size, uint8_t type);
FRESULT store_image(uint8_t* data, uint8_t data_size);
FRESULT unmount_SD();

#endif /* SRC_OBC_INTERFACE_H_ */
