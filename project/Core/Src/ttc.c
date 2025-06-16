#include "ttc.h"
#include "ttc_interface.h"
#include "obc.h"
#include "cmsis_os.h"

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    while (1) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & CAMERA & INFO) {
            // Call transmit function with pointer to camera data in flash
        	generatepacket(PayloadType./*CAMERA_1/2_MF*/, /*Pointer to cam data*/, /*Cam data length*/);
        	transmit();
        }

        if (received_notification & CAMERA & ERROR) {
        	// Tell ground station there's an error
        	generatepacket(PayloadType.ERROR_PER, CAMERA, 4);
        	transmit();
        }

        if (received_notification & IDLE & WARNING) {
        	// Can't transmit in IDLE mode
        	// Do nothing
        }

        if (received_notification & LOW_POWER & WARNING) {
        	// Can't take photos in LOW POWER mode
        	// Tell ground station low power, no picture
        	// Call transmit
        	generatepacket(PayloadType.ERROR_LP, Null, 0);
        	transmit();
        }

        if (received_notification & REQUEST & GPS) {
        	// Call interface function to read GPS
        	int gps_result = gps_hl();

			// Send task notification to the OBC
			if(gps_result == 0) {
				xTaskNotify(obc_notifications, INFO & GPS, eSetValueWithOverwrite);
				//transmit all data from flash
				generatepacket(PayloadType.TELEM, &TELEM_FLASH_MEM, TELEM_DATA_LEN);
				transmit();
			} else {
				xTaskNotify(obc_notifications, ERROR & GPS, eSetValueWithOverwrite);
				generatepacket(PayloadType.ERROR_PER, GPS, 4);
				transmit();
			}
        }
    }
}
// If received packet is request for camera data, Notify OBC that camera data is needed
	// this should go in receive function:
	// ulTaskNotify(obc_notifications, REQUEST_CAMERA, eSetValueWithOverwrite);
