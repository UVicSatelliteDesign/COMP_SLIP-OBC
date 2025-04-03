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
        }

        if (received_notification & CAMERA_ERROR) {
        	// Tell ground station there's an error
        }

        if (received_notification & IDLE_WARNING) {
        	// Can't transmit in IDLE mode
        	// Do nothing
        }

        if (received_notification & LOW_POWER_WARNING) {
        	// Can't take photos in LOW POWER mode
        	// Tell ground station low power, no picture
        	// Call transmit
        }

        if (received_notification & REQUEST_GPS) {
        	// Call interface function to read GPS
        	int gps_result = 0; //gps_hl();

			// Send task notification to the OBC
			if(gps_result == 0) {
				ulTaskNotify(obc_notifications, GPS_READY, eSetValueWithOverwrite);
				//transmit all data from flash
			} else {
				ulTaskNotify(obc_notifications, GPS_ERROR, eSetValueWithOverwrite);
			}
        }

        // if status change notif send to ground station
    }
}
// If request for camera data, Notify OBC that camera data is needed
	// this should go in receive function:
	// ulTaskNotify(obc_notifications, REQUEST_CAMERA, eSetValueWithOverwrite);

void transmit_camera(/*pointer to camera data?, offset*/) {
	// Call if acknowledgment of camera data + request for next bytes received
	// call transmit with pointer to camera data + offset, type, total length
}
