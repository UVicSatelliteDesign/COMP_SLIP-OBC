#include "obc.h"
#include "obc_interface.h"
#include "main.h"
#include "time.h"

int mode = IDLE;
HAL_StatusTypeDef status;
BatteryData battery_data;

void obc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    for (;;) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        // Image requested from TTC
        if (received_notification & REQUEST & CAMERA) {
        	if (mode == IDLE) {
        		xTaskNotify(ttc_notifications, WARNING & IDLE, eSetValueWithOverwrite);
        		store_data("Idle image request", T_WARNING);

        	} else if (mode == LOW_POWER) {
        		xTaskNotify(ttc_notifications, WARNING & LOW_POWER, eSetValueWithOverwrite);
        		store_data("Low power image request", T_WARNING);

        	} else {
        		// Take a picture
        		freeImageBuffer();
        		status = capture_snapshot();

        		if (status == HAL_OK) {
        			// Store image
        			save_image_to_flash();
        			float image_buffer[MAX_IMAGE_BUFFER_SIZE];
        			Flash_Read_Data(/* Image address, image_buffer, image size */);
        			store_image(image_buffer);

        			// Notify TTC there is an image
        			xTaskNotify(ttc_notifications, INFO & CAMERA, eSetValueWithOverwrite);
        		} else {

        			//Error collecting image
        			xTaskNotify(ttc_notifications, ERROR & CAMERA, eSetValueWithOverwrite);
        			store_data("Camera error", T_ERROR);
				}
        	}
        }

        // New GPS data in flash
        if (received_notification & INFO & GPS) {

        	SensorsData sensor_data;
        	BatteryData battery_data;
        	// TODO: GPS

        	sensor_data = load_sensor_data_from_flash();
        	battery_data = load_battery_data_from_flash();
        	// TODO: GPS

        	// Save telemetry to memory
        	store_data((uint8_t*)&sensor_data, T_DATA);
        	store_data((uint8_t*)&battery_data, T_DATA);
        	// TODO: GPS
        }

        if (received_notification & ERROR & GPS) {
        	store_data("GPS error", T_ERROR);
        }

        if (received_notification & REQUEST & LOW_POWER) {
        	set_mode(LOW_POWER); // Ground station requested low power
        }

        if (received_notification & REQUEST & NOMINAL) {
        	set_mode(NOMINAL); // Ground station requested nominal
        }

        received_notification = 0;
    }
}

// Collect data
void data_task(void *vpParameters) {
	SensorsData sensor_data;
	for (;;) {
		if (mode != IDLE) {
			sensor_data = read_sensors();
			save_battery_data_to_flash(&battery_data); // Continuously updated in low_power_task
			save_sensor_data_to_flash(&sensor_data);
			xTaskNotify(ttc_notifications, REQUEST & GPS, eSetValueWithOverwrite); // Request GPS data
		}

		if (mode == NOMINAL) {
			vTaskDelay(pdMS_TO_TICKS(NOMINAL_INTERVAL)); // Wait nominal time
		} else if ( mode == LOW_POWER) {
			vTaskDelay(pdMS_TO_TICKS(LOW_POWER_INTERVAL)); // Wait low power time
		}
	}
}

// Continuously check battery for low power mode
void low_power_task(void *vpParameters) {
	int current_time = 0;
	int previous_time = 0;
	for (;;) {
		current_time = HAL_GetTick();
		battery_data = get_battery_data(current_time-previous_time); // Use the amount of time since the last call
		previous_time = HAL_GetTick();
		if (mode != LOW_POWER && battery_data.state_of_charge < LOW_POWER_THRESHOLD) { // If power drops too low switch to low power
		set_mode(LOW_POWER);
		}
	}
}

// Change mode and notify TTC
void set_mode(int m) {
	mode = m;
	if (m == NOMINAL) {
		xTaskNotify(ttc_notifications, INFO & NOMINAL, eSetValueWithOverwrite);
	} else if (m == LOW_POWER) {
		xTaskNotify(ttc_notifications, INFO & LOW_POWER, eSetValueWithOverwrite);
	}
}
