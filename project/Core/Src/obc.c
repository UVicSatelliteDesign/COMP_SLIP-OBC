#include "obc.h"
#include "obc_interface.h"
#include "main.h"

int mode = IDLE;
HAL_StatusTypeDef status;

void obc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    for (;;) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        // Image requested from TTC
        if (received_notification & REQUEST_CAMERA) {
        	if (mode == IDLE) {
        		xTaskNotify(ttc_notifications, IDLE_WARNING, eSetValueWithOverwrite);
        		store_data("Idle image request", T_WARNING);

        	} else if (mode == LOW_POWER) {
        		xTaskNotify(ttc_notifications, LOW_POWER_WARNING, eSetValueWithOverwrite);
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
        			xTaskNotify(ttc_notifications, CAMERA_READY, eSetValueWithOverwrite);
        		} else {

        			//Error collecting image
        			xTaskNotify(ttc_notifications, CAMERA_ERROR, eSetValueWithOverwrite);
        			store_data("Camera error", T_ERROR);
				}
        	}
        }

        // New GPS data in flash
        if (received_notification & GPS_READY) {
        	// TODO: Retrieve GPS, sensor, and BMS data from flash and write to external memory

        	float bms_buffer[MAX_DATA_SIZE];
        	float sensor_buffer[MAX_DATA_SIZE];
        	float gps_buffer[MAX_DATA_SIZE];

        	// Collect telemetry data
        	Flash_Read_Data(/* BMS address, bms_buffer, BMS size */);
        	Flash_Read_Data(/* Sensor address, sensor_buffer, Sensor size */);
        	Flash_Read_Data(/* GPS address, gps_buffer, GPS size */);

        	// Save telemetry to memory
        	store_data(/* bms_buffer, T_DATA */);
        	store_data(/* sensor_buffer, T_DATA */);
        	store_data(/* gps_buffer, T_DATA */);
        }

        if (received_notification & GPS_ERROR) {
        	store_data("GPS error", T_ERROR);
        }

        if (received_notification & SET_LOW_POWER) {
        	set_mode(LOW_POWER);
        }

        if (received_notification & SET_NOMINAL) {
        	set_mode(NOMINAL);
        }

        received_notification = 0;
    }
}

void data_task(void *vpParameters) {
	for (;;) {
		if (mode != IDLE) {
			read_bms();
			read_sensors();
			xTaskNotify(ttc_notifications, REQUEST_GPS, eSetValueWithOverwrite);
		}

		if (mode == NOMINAL) {
			vTaskDelay(pdMS_TO_TICKS(NOMINAL_INTERVAL)); // eg. 1000 = 1 second
		} else if ( mode == LOW_POWER) {
			vTaskDelay(pdMS_TO_TICKS(LOW_POWER_INTERVAL));
		}
	}
}

void low_power_task(void *vpParameters) {
	for (;;) {
		if (mode != LOW_POWER && battery_health() < LOW_POWER_THRESHOLD) {
		set_mode(LOW_POWER);
		}
	}
}

void set_mode(int m) {
	mode = m;
	if (m == NOMINAL) {
		xTaskNotify(ttc_notifications, MODE_NOMINAL, eSetValueWithOverwrite);
	} else if (m == LOW_POWER) {
		xTaskNotify(ttc_notifications, MODE_LOW_POWER, eSetValueWithOverwrite);
	}
}
