#include "ttc.h"
#include "obc.h"
#include "cmsis_os.h"

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    while (1) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & CAMERA_READY) {
            // TODO
        }

        if (received_notification & CAMERA_ERROR) {
        	// TODO
        }

        if (received_notification & REQUEST_GPS) {
        	// Call interface function to read GPS
			// Store GPS data

			// Send task notification to the OBC
			if(/*GPS data read correctly*/) {
				ulTaskNotify(obc_notifications, GPS_READY, eSetValueWithOverwrite);
			} else {
				ulTaskNotify(obc_notifications, GPS_ERROR, eSetValueWithOverwrite);
			}
        }
    }
}
// send to obc: REQUEST_CAMERA, GPS_READY, GPS_ERROR
