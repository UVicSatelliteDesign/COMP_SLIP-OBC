#include "obc.h"
#include "obc_interface.h"
#include "camera.h"
#include "main.h"
#include "time.h"

int mode = NOMINAL_MODE; // Start the payload in nominal mode
HAL_StatusTypeDef status;
BatteryData battery_data;
Camera_t camera1;
Camera_t camera2;

extern SensorsData sensor_backup;
extern BatteryData battery_backup;

extern SemaphoreHandle_t image_mutex;

void obc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    uint8_t camera1_buffer[MAX_IMAGE_BUFFER_SIZE];
    uint8_t camera2_buffer[MAX_IMAGE_BUFFER_SIZE];

    configure_camera1(&camera1, camera1_buffer);
    configure_camera2(&camera2, camera2_buffer);

    camera_init(&camera1);
    camera_init(&camera2);

    for (;;) {
    	// Check for a notification from the TTC
        received_notification = ulTaskNotifyTake(pdTRUE, 0); // Set the notification value to 0 with 0s timeout

        // Image requested from TTC
        if (received_notification & REQUEST & CAMERA) {
        	if (mode == LOW_POWER) {
        		// Send Low power warning
        		xTaskNotify(ttc_notifications, WARNING & LOW_POWER, eSetValueWithOverwrite);
        		store_data((uint8_t*)"Low power image request", T_WARNING);

        	} else {
        		// Take a picture
        		if (xSemaphoreTake(image_mutex, portMAX_DELAY) == pdTRUE) {

					if (received_notification & SUB_1) {
						freeImageBuffer(&camera1);
						status = capture_snapshot(&camera1);
						if (status == HAL_OK) {
							save_image_to_flash(&camera1, CAMERA_FLASH_SECTOR);
							store_image(camera1.imageBuffer);
						} else {
							xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_1, eSetValueWithOverwrite);
							store_data((uint8_t*)"Camera 1 error", T_ERROR);
						}
					} else if (received_notification & SUB_2) {
						freeImageBuffer(&camera2);
						status = capture_snapshot(&camera2);
						if (status == HAL_OK) {
							save_image_to_flash(&camera2, CAMERA_FLASH_SECTOR);
							store_image(camera2.imageBuffer);
						} else {
							xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_2, eSetValueWithOverwrite);
							store_data((uint8_t*)"Camera 2 error", T_ERROR);
						}
					}
					xSemaphoreGive(image_mutex);
        		}
        	}
        }

        // New GPS data in flash
        if (received_notification & INFO & GPS) {

        	float altimeter_data;

        	load_sensor_data_from_flash();
        	load_battery_data_from_flash();
        	flash_read_data(FLASH_TYPE_ALTIMETER, altimeter_data, sizeof(float), sector, expected_magic) // TODO: sector and magic

        	// Save telemetry to memory
        	store_data((uint8_t*)&sensor_data, T_DATA);
        	store_data((uint8_t*)&battery_data, T_DATA);
        	store_data((uint8_t)*&altimeter_data, T_DATA);
        }

        if (received_notification & ERROR & GPS) {
        	store_data((uint8_t*)"GPS error", T_ERROR);
        }

        if (received_notification & REQUEST & LOW_POWER) {
        	set_mode(LOW_POWER_MODE); // Ground station requested low power
        }

        if (received_notification & REQUEST & NOMINAL) {
        	set_mode(NOMINAL_MODE); // Ground station requested nominal
        }

        received_notification = 0;
    }
}

// Collect data
void data_task(void *vpParameters) {
	float altimeter_data;
	SensorsData sensor_data;
	for (;;) {
		sensor_data = read_sensors();
		altimeter_data = altimeter_read();
		save_battery_data_to_flash(&battery_data); // Continuously updated in low_power_task
		save_sensor_data_to_flash(&sensor_data);
		flash_write_data(FLASH_TYPE_ALTIMETER, &altiemeter_data, sizeof(float), sector) // TODO: Add sector
		xTaskNotify(ttc_notifications, REQUEST & GPS, eSetValueWithOverwrite); // Request GPS data

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
		if (mode != LOW_POWER_MODE && battery_data.state_of_charge < LOW_POWER_THRESHOLD) { // If power drops too low switch to low power
		set_mode(LOW_POWER_MODE);

		// TODO: if (on ground) {
		// set_mode(LOW_POWER_MODE);
		}
	}
}

// Take an image at an interval defined by IMAGE_INTERVAL
void image_task(void *vpParameters) {
	if (xSemaphoreTake(image_mutex, portMAX_DELAY) == pdTRUE) {
		if (mode == NOMINAL) {

			freeImageBuffer(&camera1);
			status = capture_snapshot(&camera1);
			if (status == HAL_OK) {
				save_image_to_flash(&camera1, CAMERA_FLASH_SECTOR);
				store_image(camera1.imageBuffer);
			} else {
				xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_1, eSetValueWithOverwrite);
				store_data((uint8_t*)"Camera 1 error", T_ERROR);
			}
			xSemaphoreGive(image_mutex);
		}
	}



	if (xSemaphoreTake(image_mutex, portMAX_DELAY) == pdTRUE) {
		if (mode == NOMINAL) {

			freeImageBuffer(&camera2);
			status = capture_snapshot(&camera2);
			if (status == HAL_OK) {
				save_image_to_flash(&camera2, CAMERA_FLASH_SECTOR);
				store_image(camera2.imageBuffer);
			} else {
				xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_2, eSetValueWithOverwrite);
				store_data((uint8_t*)"Camera 2 error", T_ERROR);
			}
			xSemaphoreGive(image_mutex);
		}
	}

	vTaskDelay(pdMS_TO_TICKS(IMAGE_INTERVAL));
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
