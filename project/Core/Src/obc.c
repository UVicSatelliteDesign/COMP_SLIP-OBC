#include "obc.h"
#include "cmsis_os.h"

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
        		ulTaskNotify(ttc_notification, IDLE_WARNING, eSetValueWithOverwrite);
        	} else if (mode == LOW_POWER) {
        		ulTaskNotify(ttc_notification, LOW_POWER_WARNING, eSetValueWithOverwrite);
        	} else {
        		freeImageBuffer();
        		status = capture_snapshot();
        		if (status == HAL_OK) {
        			save_image_to_flash();
        			float image_buffer[MAX_IMAGE_BUFFER_SIZE];
        			Flash_Read_Data(/* Image address, image_buffer, image size */);
        			store_image(image_buffer);
        			ulTaskNotify(ttc_notifications, CAMERA_READY, eSetValueWithOverwrite);
        		} else {
        			ulTaskNotify(ttc_notifications, CAMERA_ERROR, eSetValueWithOverwrite);
				}
        	}
        }

        if (received_notification & GPS_READY) {
        	// TODO: Retrieve GPS, sensor, and BMS data from flash and write to external memory

        	float bms_buffer[MAX_DATA_SIZE];
        	float sensor_buffer[MAX_DATA_SIZE];
        	float gps_buffer[MAX_DATA_SIZE];

        	Flash_Read_Data(/* BMS address, bms_buffer, BMS size */);
        	Flash_Read_Data(/* Sensor address, sensor_buffer, Sensor size */);
        	Flash_Read_Data(/* GPS address, gps_buffer, GPS size */);

        	store_data(/* bms_buffer, T_DATA */);
        	store_data(/* sensor_buffer, T_DATA */);
        	store_data(/* gps_buffer, T_DATA */);
        }

        if (received_notification & GPS_ERROR) {
        	char* err_msg = "GPS error";

        	store_data(err_msg, T_ERROR);
        }

        if (received_notification & LOW_POWER_MODE) {
        	mode = LOW_POWER;
        }

        if (received_notification & NOMINAL) {
        	mode = NOMINAL;
        }

        received_notification = 0;
    }
}

void data_task(void *vpParameters) {
	for (;;) {
		if (mode != IDLE) {
			read_bms();
			read_sensors();
			ulTaskNotify(ttc_notifications, REQUEST_GPS, eSetValueWithOverwrite);
		}

		if (mode == NOMINAL) {
			vTaskDelay(pdMS_TO_TICKS(20000)); // eg. 1000 = 1 second
		} else if ( mode == LOW_POWER) {
			vTaskDelay(pdMS_TO_TICKS(60000)); // Replace these with define in .h
		}
	}
}

void low_power_task(void *vpParameters) {
	for (;;) {
		if (mode != LOW_POWER && battery_health() < LOW_POWER_THRESHOLD) {
		mode = LOW_POWER;
		}
	}
}
