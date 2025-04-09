#include "ttc.h"
#include "obc.h"
#include "cmsis_os.h"

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    while (1) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & CAMERA_READY) {
            // Call transmit function with pointer to camera data in flash
        	transmit(FLASH_CAMERA_DATA, CAMERA_DATA_TYPE, CAMERA_DATA_LEN);
        }

        if (received_notification & CAMERA_ERROR) {
        	// Tell ground station there's an error
        	transmit_camera_error();
        }

        if (received_notification & IDLE_WARNING) {
        	// Can't transmit in IDLE mode
        	// Do nothing
        }

        if (received_notification & LOW_POWER_WARNING) {
        	// Can't take photos in LOW POWER mode
        	// Tell ground station low power, no picture
        	transmit_low_power(); // Call transmit
        }

        if (received_notification & REQUEST_GPS) {
        	// Call interface function to read GPS
        	int gps_result = gps_hl();

			// Send task notification to the OBC
			if(gps_result == 0) {
				ulTaskNotify(obc_notifications, GPS_READY, eSetValueWithOverwrite);
				//transmit all data from flash
				transmit(FLASH_SENSOR_DATA, SENSOR_DATA_TYPE, SENSOR_DATA_LEN);
			} else {
				ulTaskNotify(obc_notifications, GPS_ERROR, eSetValueWithOverwrite);
			}
        }
    }
}
// If received packet is request for camera data, Notify OBC that camera data is needed
	// this should go in receive function:
	// ulTaskNotify(obc_notifications, REQUEST_CAMERA, eSetValueWithOverwrite);

void transmit_camera(int offset) {
	// Call if acknowledgment of camera data + request for next bytes received
	// call transmit with pointer to camera data + offset, type, total length
	transmit(FLASH_CAMERA_DATA + offset, CAMERA_DATA_TYPE, CAMERA_DATA_LEN);
}
