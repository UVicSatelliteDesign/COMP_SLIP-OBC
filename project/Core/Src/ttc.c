#include "ttc.h"
#include "ttc_interface.h"
#include "main.h"
#include "cmsis_os.h"

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    for (;;) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & INFO & CAMERA) {
            // Call transmit function with pointer to camera data in flash
        	generatepacket(/*CAMERA_1/2_MF*/, /*Pointer to cam data*/, /*Cam data length*/);
        }

        if (received_notification & ERROR & CAMERA) {
        	// Tell ground station there's an error
        	generatepacket(ERROR_PER, /*Error message*/, /*Error msg length*/);
        }

        if (received_notification & WARNING & IDLE) {
        	// Can't transmit in IDLE mode
        	// Do nothing
        }

        if (received_notification & WARNING & LOW_POWER) {
        	// Can't take photos in LOW POWER mode
        	// Tell ground station low power, no picture
        	// Call transmit
        	generatepacket(ERROR_LP, /*low power*/, /*msg len*/);
        }

        if (received_notification & REQUEST & GPS) {
        	// Call interface function to read GPS
        	int gps_result = gps_hl();

			// Send task notification to the OBC
			if(gps_result == 0) {
				ulTaskNotify(obc_notifications, READY & GPS, eSetValueWithOverwrite);
				//transmit all data from flash
				generatepacket(TELEMETRY, &TELEM_FLASH_MEM, TELEM_DATA_LEN);
			} else {
				ulTaskNotify(obc_notifications, ERROR & GPS, eSetValueWithOverwrite);
			}
        }
        received_notification = 0;
    }
}
// If received packet is request for camera data, Notify OBC that camera data is needed
	// this should go in receive function:
	// ulTaskNotify(obc_notifications, REQUEST_CAMERA, eSetValueWithOverwrite);
