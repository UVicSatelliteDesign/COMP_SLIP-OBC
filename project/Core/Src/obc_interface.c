#include "obc_interface.h"
#include <math.h>

/*
xTaskCreate(BMS_Task, "BMS", 256, NULL, 2, NULL);
vTaskStartScheduler(); // Start FreeRTOS scheduler
*/
// Put these in main.c before starting FreeRTOS

void init_bms() {
    HAL_ADC_Start(&hadc1);
}

// Reads battery voltage from ADC
float read_battery_voltage() {
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    return (raw / 4095.0) * 3.3 * 11; // Assuming voltage divider ratio of 11
}

// Reads battery current from ADC
float read_battery_current() {
    HAL_ADC_PollForConversion(&hadc2, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc2);
    return (raw / 4095.0) * 3.3 / 0.01; // Assuming 0.01 Ohm sense resistor
}

// Reads battery temperature from ADC
float read_battery_temperature() {
    HAL_ADC_PollForConversion(&hadc3, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc3);
    float resistance = (10000.0 * raw) / (4095.0 - raw); // NTC Thermistor calculation
    return 1.0 / (0.001129148 + (0.000234125 * log(resistance))) - 273.15;
}

// Estimates battery State of Charge (SoC) based on current draw
float calculate_state_of_charge(float current, float dt) {
    static float accumulated_charge = 0;
    float battery_capacity_Ah = 3.0; // Example: 3000mAh (3Ah) battery

    accumulated_charge += (current * dt) / 3600.0; // Convert mA to Ah
    float soc = (1.0 - (accumulated_charge / battery_capacity_Ah)) * 100.0;

    return soc > 0 ? soc : 0; // Prevent negative SoC
}

// Calculates power usage
float calculate_power_usage(float voltage, float current) {
    return voltage * current; // Power in Watts (P = V * I)
}

// Calculates total energy usage over time
float calculate_energy_usage(float power, float dt) {
    static float total_energy = 0; // Stores total energy in Wh
    total_energy += (power * dt) / 3600.0; // Convert seconds to hours
    return total_energy;
}

// Estimates remaining battery life
float estimate_battery_life(float state_of_charge, float avg_power_draw) {
    float battery_capacity_Wh = 50.0; // Example: 50Wh battery
    float remaining_energy = (state_of_charge / 100.0) * battery_capacity_Wh;

    if (avg_power_draw == 0) return -1; // Avoid divide-by-zero error
    return remaining_energy / avg_power_draw; // Returns remaining battery life in hours
}

// Collects battery data into a struct
BatteryData get_battery_data(float dt) {
    BatteryData data;
    data.voltage = read_battery_voltage();
    data.current = read_battery_current();
    data.temperature = read_battery_temperature();
    data.state_of_charge = calculate_state_of_charge(data.current, dt);
    data.power_usage = calculate_power_usage(data.voltage, data.current);
    data.total_energy_used = calculate_energy_usage(data.power_usage, dt);
    data.estimated_life = estimate_battery_life(data.state_of_charge, data.power_usage);

    return data;
}
