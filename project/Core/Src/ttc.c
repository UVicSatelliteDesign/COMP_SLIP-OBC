#include "ttc.h"
#include "ttc_interface.h"
#include "obc.h"
#include "cmsis_os.h"

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    while (1) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & CAMERA_READY) {
            // Call transmit function with pointer to camera data in flash
        	generatepacket(/*CAMERA_1/2_MF*/, /*Pointer to cam data*/, /*Cam data length*/);
        }

        if (received_notification & CAMERA_ERROR) {
        	// Tell ground station there's an error
        	generatepacket(ERROR_PER, /*Error message*/, /*Error msg length*/);
        }

        if (received_notification & IDLE_WARNING) {
        	// Can't transmit in IDLE mode
        	// Do nothing
        }

        if (received_notification & LOW_POWER_WARNING) {
        	// Can't take photos in LOW POWER mode
        	// Tell ground station low power, no picture
        	// Call transmit
        	generatepacket(ERROR_LP, /*low power*/, /*msg len*/);
        }

        if (received_notification & REQUEST_GPS) {
        	// Call interface function to read GPS
        	int gps_result = gps_hl();

			// Send task notification to the OBC
			if(gps_result == 0) {
				ulTaskNotify(obc_notifications, GPS_READY, eSetValueWithOverwrite);
				//transmit all data from flash
				generatepacket(TELEMETRY, &TELEM_FLASH_MEM, TELEM_DATA_LEN);
			} else {
				ulTaskNotify(obc_notifications, GPS_ERROR, eSetValueWithOverwrite);
			}
        }
    }
}
// If received packet is request for camera data, Notify OBC that camera data is needed
	// this should go in receive function:
	// ulTaskNotify(obc_notifications, REQUEST_CAMERA, eSetValueWithOverwrite);
