#include "obc.h"
#include "obc_interface.h"
#include "camera.h"
#include "main.h"
#include "time.h"
#include "flash_interface.h"

int mode = NOMINAL_MODE; // Start the payload in nominal mode

HAL_StatusTypeDef status;
Camera_t camera1;
Camera_t camera2;
float initial_altitude; // Altitude at launch

BatteryData battery_data; // Struct for reading from battery
extern SensorsData sensor_backup; // Struct for reading from flash
extern BatteryData battery_backup; // Struct for reading from flash

extern SemaphoreHandle_t image_mutex;

void obc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    uint8_t camera1_buffer[MAX_IMAGE_BUFFER_SIZE];
    uint8_t camera2_buffer[MAX_IMAGE_BUFFER_SIZE];

    configure_camera1(&camera1, camera1_buffer);
    configure_camera2(&camera2, camera2_buffer);

    camera_init(&camera1);
    camera_init(&camera2);
    altimeter_init();
    init_sensors();
    init_bms();

    initial_altitude = altimeter_read();


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
        		if (xSemaphoreTake(image_mutex, portMAX_DELAY) == pdTRUE) { // Camera mutex

					if (received_notification & SUB_1) { // Camera 1

						// Switch to camera 1
						SWITCH_CAMERA(camera1, 1);
						SWITCH_CAMERA(camera2, 0);
						freeImageBuffer(&camera1);

						status = capture_snapshot(&camera1); // Take picture
						if (status == HAL_OK) {
							save_image_to_flash(&camera1, CAMERA_FLASH_SECTOR);
							flash_write(camera1.actualImageSize, sizeof(uint16_t), FLASH_SECTOR_IMAGE_DATA_LEN, IMAGE_DATA_LEN_OFFSET);
							store_image(camera1.imageBuffer);

							// Notify TTC
							xTaskNotify(ttc_notifications, INFO & CAMERA & SUB_1, eSetValueWithOverwrite);

						} else { // Error occurred
							xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_1, eSetValueWithOverwrite);
							store_data((uint8_t*)"Camera 1 error", T_ERROR);
						}
					} else if (received_notification & SUB_2) { // Camera 2

						// Switch to camera 2
						SWITCH_CAMERA(camera1, 0);
						SWITCH_CAMERA(camera2, 1);
						freeImageBuffer(&camera2);

						status = capture_snapshot(&camera2); // Take picture
						if (status == HAL_OK) {
							save_image_to_flash(&camera2, CAMERA_FLASH_SECTOR);
							flash_write(camera2.actualImageSize, sizeof(uint16_t), FLASH_SECTOR_IMAGE_DATA_LEN, IMAGE_DATA_LEN_OFFSET);
							store_image(camera2.imageBuffer);

							// Notify TTC
							xTaskNotify(ttc_notifications, INFO & CAMERA & SUB_2, eSetValueWithOverwrite);

						} else { // Error occurred
							xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_2, eSetValueWithOverwrite);
							store_data((uint8_t*)"Camera 2 error", T_ERROR);
						}
					}
					xSemaphoreGive(image_mutex);
        		}
        	}
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

		// Collect data
		sensor_data = read_sensors();
		altimeter_data = altimeter_read();

		// Clear flash
		flash_clear(FLASH_SECTOR_BATTERY);
		flash_clear(FLASH_SECTOR_ALTIMETER);

		// Save data to flash
		save_battery_data_to_flash(&battery_data); // Continuously updated in low_power_task
		save_sensor_data_to_flash(&sensor_data);
		flash_write(&altimeter_data, sizeof(float), FLASH_SECTOR_ALTIMETER, 0);

		// Tell TTC that there is data to be transmitted
		xTaskNotify(ttc_notifications, INFO & SENSORS, eSetValueWithOverwrite);

		// Store data in memory
		store_data((uint8_t*)&sensor_backup, T_DATA);
		store_data((uint8_t*)&battery_data, T_DATA);
		store_data((uint8_t*)&altimeter_data, T_DATA);

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
	int above_altitude = 0;
	for (;;) {
		current_time = HAL_GetTick();
		battery_data = get_battery_data(current_time - previous_time); // Use the amount of time since the last call
		previous_time = HAL_GetTick();
		if (mode != LOW_POWER_MODE && battery_data.state_of_charge < LOW_POWER_THRESHOLD) {
			set_mode(LOW_POWER_MODE); // If power drops too low switch to low power
		}
		if (!above_altitude && altimeter_read() > initial_altitude + 30) { // Leave room for error and elevation change
			above_altitude = 1; // Make sure the payload has launched before checking altitude for low power
		}
		if ( mode != LOW_POWER_MODE && above_altitude && altimeter_read() < initial_altitude + 20) { // Leave room for elevation change
			set_mode(LOW_POWER_MODE); // Low power if close to the ground and already completed its launch
		}

		if (read_TTC_temperature() > TTC_TEMPERATURE_THRESHOLD) {
			set_mode(LOW_POWER_MODE); // If TTC temperature gets too hot, go into low power
		}
	}
}

// Take an image at an interval defined by IMAGE_INTERVAL
void image_task(void *vpParameters) {
	if (xSemaphoreTake(image_mutex, portMAX_DELAY) == pdTRUE) { // Mutex for camera 1
		if (mode == NOMINAL) {

			// Switch to camera 1
			SWITCH_CAMERA(camera1, 1);
			SWITCH_CAMERA(camera2, 0);

			freeImageBuffer(&camera1);
			status = capture_snapshot(&camera1); // Take picture

			if (status == HAL_OK) {
				store_image(camera1.imageBuffer);

			} else { // Error occurred
				xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_1, eSetValueWithOverwrite);
				store_data((uint8_t*)"Camera 1 error", T_ERROR);
			}
			xSemaphoreGive(image_mutex);
		}
	}



	if (xSemaphoreTake(image_mutex, portMAX_DELAY) == pdTRUE) { // Mutex for camera 2
		if (mode == NOMINAL) {

			// Switch to camera 2
			SWITCH_CAMERA(camera1, 2);
			SWITCH_CAMERA(camera2, 1);

			freeImageBuffer(&camera2);
			status = capture_snapshot(&camera2); // Take picture

			if (status == HAL_OK) {
				store_image(camera2.imageBuffer);

			} else { // Error occurred
				xTaskNotify(ttc_notifications, ERROR & CAMERA & SUB_2, eSetValueWithOverwrite);
				store_data((uint8_t*)"Camera 2 error", T_ERROR);
			}
			xSemaphoreGive(image_mutex);
		}
	}
	vTaskDelay(pdMS_TO_TICKS(IMAGE_INTERVAL)); // Wait IMAGE_INTERVAL ticks
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
