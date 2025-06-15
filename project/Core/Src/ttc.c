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

void receive() {
	/*
	 * Read from queue of bits:
	 *
	 * Length -> 1 byte
	 * Payload type -> 1 byte
	 * Data -> variable length (length - 2 [- 3])
	 * Offset -> 3 bytes
	 * Seq num -> 2 bytes
	*/
	uint8_t data_buffer[128];
	osStatus_t status = osMessageQueueGet(receivequeueHandle, &data_buffer, NULL, 0U);   // wait for message
	if (status != osOK) {
		ulTaskNotify(obc_notifications, ERROR, eSetValueWithOverwrite);
		return; // Error
	}
	uint8_t packet_length = data_buffer[0];
	PayloadType packet_type = (PayloadType) data_buffer[1];

	switch(packet_type) {
		case PayloadType.PING:
			// Acknowledge ping
			generatepacket(PayloadType.PING, Null, 0);
			break;
		case PayloadType.NOMINAL:
			ulTaskNotify(obc_notifications, REQUEST & NOMINAL, eSetValueWithOverwrite);
			uint16_t seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			generatepacket(PayloadType.ACK_REC_STATUS, seq_num, 2);
			transmit();
			break;
		case PayloadType.LOW_POWER:
			ulTaskNotify(obc_notifications, REQUEST & LOW_POWER, eSetValueWithOverwrite);
			uint16_t seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			generatepacket(PayloadType.ACK_REC_STATUS, seq_num, 2);
			break;
		case PayloadType.CAMERA_1_END:
			// request for image, notify OBC
			ulTaskNotify(obc_notifications, REQUEST & CAMERA, eSetValueWithOverwrite);
			break;
		case PayloadType.CAMERA_2_END:
		// request for image, notify OBC
			ulTaskNotify(obc_notifications, REQUEST & CAMERA, eSetValueWithOverwrite);
			break;
		case PayloadType.ACK_REC_CAMER:
		// set as acknowledged
		// Contains an offset
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			uint32_t acked_offset =
					(data_buffer[4] << 16) |
					(data_buffer[5] << 8) |
					(data_buffer[6]);
			uint16_t seq_num = (data_buffer[7] << 8) | (data_buffer[8]);
			break;
		case PayloadType.ACK_REC_TELEMETRY:
		// set as acknowledged
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			break;
		case PayloadType.ACK_REC_ERROR:
		// set as acknowledged
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			break;
		else:
		// Error: should not be receiving other packet types
			// Notify OBC of packet error
			ulTaskNotify(obc_notifications, ERROR, eSetValueWithOverwrite);
			break;
	}
}
