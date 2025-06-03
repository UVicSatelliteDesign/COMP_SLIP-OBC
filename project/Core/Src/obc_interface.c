#include "obc_interface.h"
#include "main.h"

#define FLASH_SAVE_ADDRESS  ((uint32_t)0x081E0000) // Example sector 7 start (adjust based on your chip)
#define FLASH_MAGIC         ((uint32_t)0xDEADBEEF)

extern ADC_HandleTypeDef hadc_voltage; // ADC handler for voltage
extern ADC_HandleTypeDef hadc_current; // ADC handler for current
extern ADC_HandleTypeDef hadc_temperature; // ADC handler for temperature

// SD card variables
FRESULT res; // FatFS result code
uint32_t byteswritten; // File write count
uint32_t bytesread; // File read count
//uint8_t wtext[] = "Example text to write"; // File write buffer
uint8_t rtext[_MAX_SS]; // File read buffer

uint16_t image_count = 0; // Keep track of images stored for file names

BatteryData battery_backup = {0};

void init_bms() {
    HAL_ADC_Start(&hadc_voltage);
    HAL_ADC_Start(&hadc_current);
    HAL_ADC_Start(&hadc_temperature);
}

float read_battery_voltage() {
    HAL_ADC_PollForConversion(&hadc_voltage, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc_voltage);
    return (raw / 4095.0f) * 3.3f * 11.0f;
}

float read_battery_current() {
    HAL_ADC_PollForConversion(&hadc_current, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc_current);
    return (raw / 4095.0f) * 3.3f / 0.01f;
}

float read_battery_temperature() {
    HAL_ADC_PollForConversion(&hadc_temperature, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc_temperature);
    float resistance = (10000.0f * raw) / (4095.0f - raw);
    return 1.0f / (0.001129148f + (0.000234125f * logf(resistance))) - 273.15f;
}

float calculate_state_of_charge(float current, float dt) {
    static float accumulated_charge = 0;
    float battery_capacity_Ah = 3.0f;
    accumulated_charge += (current * dt) / 3600.0f;
    float soc = (1.0f - (accumulated_charge / battery_capacity_Ah)) * 100.0f;
    return soc > 0 ? soc : 0;
}

float calculate_power_usage(float voltage, float current) {
    return voltage * current;
}

float calculate_energy_usage(float power, float dt) {
    static float total_energy = 0;
    total_energy += (power * dt) / 3600.0f;
    return total_energy;
}

float estimate_battery_life(float state_of_charge, float avg_power_draw) {
    float battery_capacity_Wh = 50.0f;
    float remaining_energy = (state_of_charge / 100.0f) * battery_capacity_Wh;
    return (avg_power_draw > 0) ? (remaining_energy / avg_power_draw) : -1;
}

BatteryData get_battery_data(float dt) {
    BatteryData data;
    data.voltage = read_battery_voltage();
    data.current = read_battery_current();
    data.temperature = read_battery_temperature();
    data.state_of_charge = calculate_state_of_charge(data.current, dt);
    data.power_usage = calculate_power_usage(data.voltage, data.current);
    data.total_energy_used = calculate_energy_usage(data.power_usage, dt);
    data.estimated_life = estimate_battery_life(data.state_of_charge, data.power_usage);
    data.magic = FLASH_MAGIC;
    return data;
}

void save_battery_data_to_flash(BatteryData *data) {
	HAL_FLASH_Unlock();

	    // 1. Setup flash erase configuration
	    FLASH_EraseInitTypeDef erase;
	    uint32_t pageError;

	    erase.TypeErase = FLASH_TYPEERASE_SECTORS;       // Erase by sector
	    erase.Sector = FLASH_SECTOR_7;                   // Make sure this is correct for your chip!
	    erase.NbSectors = 1;
	    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;      // 2.7V to 3.6V

	    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
	        // Handle erase error
	        HAL_FLASH_Lock();
	        return;
	    }

	    // 2. Write the data in 64-bit chunks
	    uint64_t *src = (uint64_t *)data;
	    uint32_t numWords = sizeof(BatteryData) / 8;

	    for (uint32_t i = 0; i < numWords; i++) {
	        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FLASH_SAVE_ADDRESS + (i * 8), src[i]) != HAL_OK) {
	            // Handle write error
	            HAL_FLASH_Lock();
	            return;
	        }
	    }

	    HAL_FLASH_Lock();
}

void load_battery_data_from_flash() {
    BatteryData *flash_data = (BatteryData *)FLASH_SAVE_ADDRESS;
    if (flash_data->magic == FLASH_MAGIC) {
        memcpy(&battery_backup, flash_data, sizeof(BatteryData));
    } else {
        memset(&battery_backup, 0, sizeof(BatteryData));
    }
}


// Mount SD card
FRESULT mount_SD(){
	res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
	// if (res != FR_OK){
	// 	// Error handling
	// }
	return res;
}

// Format SD card (run once)
FRESULT format_SD(){
	// TODO: check if SD card is already formatted
	res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, rtext, sizeof(rtext));
	// if (res != FR_OK){
	// 	// Error handling
	// }
	return res;
}
