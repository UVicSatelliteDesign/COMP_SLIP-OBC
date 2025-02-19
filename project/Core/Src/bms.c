/*
 * bms.c
 *
 *  Created on: Feb 18, 2025
 *      Author: lelandsion
 */

#include "bms.h"
#include <math.h>


/*
xTaskCreate(BMS_Task, "BMS", 256, NULL, 2, NULL);
vTaskStartScheduler(); // Start FreeRTOS scheduler
*/
// Put these in main two in main ^

void init_bms() {
    HAL_ADC_Start(&hadc1);
}

float read_battery_voltage() {
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    return (raw / 4095.0) * 3.3 * 11;
}

float read_battery_current() {
    HAL_ADC_PollForConversion(&hadc2, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc2);
    return (raw / 4095.0) * 3.3 / 0.01;
}

float read_battery_temperature() {
    HAL_ADC_PollForConversion(&hadc3, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc3);
    float resistance = (10000 * raw) / (4095 - raw);
    return 1 / (0.001129148 + (0.000234125 * log(resistance))) - 273.15;
}

BatteryData get_battery_data() {
    BatteryData data;
    data.voltage = read_battery_voltage();
    data.current = read_battery_current();
    data.temperature = read_battery_temperature();
    data.state_of_charge = (data.voltage - 3.4) / (4.2 - 3.4) * 100.0; // Example SoC
    data.state_of_health = 90.0; // Placeholder SoH

    return data;
}


