#include "obc.h"
#include "cmsis_os.h"

void obc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    for (;;) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & REQUEST_CAMERA) {
            int result = take_snapshot(/* TODO: Create and add camera configurations */);

            if (result == 1) {
            	ulTaskNotify(ttc_notifications, CAMERA_ERROR, eSetValueWithOverwrite);
            } else {
            	ulTaskNotify(ttc_notifications, CAMERA_READY, eSetValueWithOverwrite);

            	// TODO: Retrieve camera data from flash and write to external memory

            	uint32_t camera_buffer[IMAGE_BUFFER_SIZE];

            	Flash_Read_Data(/* Camera address, camera_buffer, camera size */);

            	store_data(/* camera_buffer */);
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

        received_notification = 0;
    }
}

void data_task(void *vpParamters) {
	for (;;) {
		read_bms();
		read_sensors();
		ulTaskNotify(ttc_notifications, REQUEST_GPS, eSetValueWithOverwrite);

		vTaskDelay(pdMS_TO_TICKS(20000)); // eg. 1000 = 1 second
	}
}
