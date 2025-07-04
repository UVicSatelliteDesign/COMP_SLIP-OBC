#include "ttc.h"
#include "ttc_interface.h"
#include "obc.h"
#include "cmsis_os.h"

uint16_t last_received_seq_num; // last received seq num ack
uint8_t ACK_RECV_TIMEOUT = 3000; // timeout before checking for an acknowledgement TODO: change to real value
int communication_status = COMM_NOMINAL; // Nominal=1, Lost=0

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

void handle_transmit() {
	/* Handle transmitting packet and retransmission */
	if(communication_status) {
		transmit();
		xTimerStart(retransmission_timer, 0); // Start retransmission timer
	}
}
void vRetransmissionTimerCallback( TimerHandle_t xTimer ) {
	// Timer callback function for handling retransmission
	// Get number of times timer has expired (saved in timer ID)
	uint32_t expiry_count = ( uint32_t ) pvTimerGetTimerID( xTimer );
	expiry_count++;

	// Compare last received seq num
	if (sequenceNum == last_acked_seq_num) {
		// Packet acknowledged yay!
		expiry_count = 0;
		vTimerSetTimerID( xTimer, ( void * ) expiry_count );
		xTimerStop(xTimer, 0);
		return;
	}
	// If not acked, check if we have more attempts or not
	if (expiry_count == MAX_ATTEMPTS) {
		// Enter lost state
		communication_status = COMM_LOST;
		expiry_count = 0;
		vTimerSetTimerID(xTimer, (void *) expiry_count);
		xTimerStop(xTimer, 0);
		return;
	}
	vTimerSetTimerID(xTimer, (void *) expiry_count);
	handle_transmit();
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
	// Received a packet so we should be in nominal state
	communication_status = COMM_NOMINAL;
	// Read data from buffer
	uint8_t data_buffer[128];
	osStatus_t status = osMessageQueueGet(receivequeueHandle, &data_buffer, NULL, 0U);   // wait for message
	if (status != osOK) {
		xTaskNotify(obc_notifications, ERROR, eSetValueWithOverwrite);
		return; // Error
	}
	uint8_t packet_length = data_buffer[0];
	PayloadType packet_type = (PayloadType) data_buffer[1];

	switch(packet_type) {
		case PayloadType.PING:
			// Acknowledge ping
			generatepacket(PayloadType.PING, Null, 0);
			handle_transmit();
			break;
		case PayloadType.NOMINAL:
			xTaskNotify(obc_notifications, REQUEST & NOMINAL, eSetValueWithOverwrite);
			uint16_t seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			generatepacket(PayloadType.ACK_REC_STATUS, seq_num, 2);
			handle_transmit();
			break;
		case PayloadType.LOW_POWER:
			xTaskNotify(obc_notifications, REQUEST & LOW_POWER, eSetValueWithOverwrite);
			uint16_t seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			generatepacket(PayloadType.ACK_REC_STATUS, seq_num, 2);
			handle_transmit();
			break;
		case PayloadType.CAMERA_1_END:
			// request for image, notify OBC
			xTaskNotify(obc_notifications, REQUEST & CAMERA & SUB_1, eSetValueWithOverwrite);
			break;
		case PayloadType.CAMERA_2_END:
			// request for image, notify OBC
			xTaskNotify(obc_notifications, REQUEST & CAMERA & SUB_2, eSetValueWithOverwrite);
			break;
		case PayloadType.ACK_REC_CAMER:
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			uint8_t camera = data_buffer[4];
			// Contains an offset
			uint32_t acked_offset =
					(data_buffer[5] << 16) |
					(data_buffer[6] << 8) |
					(data_buffer[7]);
			uint16_t seq_num = (data_buffer[8] << 8) | (data_buffer[9]);
			// set as acknowledged
			last_received_seq_num = acked_seq_num;
			packageAndSendChunks(PayloadType.CAMERA, /*cam data pointer*/, /*full data len*/, acked_seq_num, acked_offset);
			handle_transmit();
			break;
		case PayloadType.ACK_REC_TELEMETRY:
			// set as acknowledged
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			last_received_seq_num = acked_seq_num;
			break;
		case PayloadType.ACK_REC_ERROR:
			// set as acknowledged
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			last_received_seq_num = acked_seq_num;
			break;
		case PayloadType.ERROR_DUP:
			// last packet was received twice by GS
			uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
			last_received_seq_num = acked_seq_num;
			break;
		else:
		// Error: should not be receiving other packet types
			// Notify OBC of packet error
			xTaskNotify(obc_notifications, ERROR, eSetValueWithOverwrite);
			break;
	}
}
