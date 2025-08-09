#include "ttc.h"
#include "ttc_interface.h"
#include "obc_interface.h"
#include "flash_interface.h"
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "stdint.h"
#include "timers.h"

#define MAX_PACKET_SIZE 128

uint16_t last_received_seq_num;			 // last received seq num ack
uint8_t ACK_RECV_TIMEOUT = 3000;		 // timeout before checking for an acknowledgement TODO: change to real value
int communication_status = COMM_NOMINAL; // Nominal=1, Lost=0

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    for (;;) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & INFO & CAMERA) {
            // Call transmit function with pointer to camera data in flash
			if (received_notification & SUB_1) {
				uint16_t image_data_len;
				// The flash_read is necessary because for persistent storage in sector 6 w/ gps
				flash_read(&image_data_len, sizeof(uint16_t), FLASH_SECTOR_IMAGE_DATA_LEN, IMAGE_DATA_LEN_OFFSET);
				packageAndSendChunks(1, FLASH_SECTOR_CAMERA, image_data_len, 0);
        		handle_transmit(0);
			} else if (received_notification & SUB_2) {
				uint16_t image_data_len;
				flash_read(&image_data_len, sizeof(uint16_t), FLASH_SECTOR_IMAGE_DATA_LEN, IMAGE_DATA_LEN_OFFSET);
				packageAndSendChunks(2, FLASH_SECTOR_CAMERA, image_data_len, 0);
				handle_transmit(0);
			}
        	
        }

        if (received_notification & ERROR & CAMERA) {
        	// Tell ground station there's an error
			if (received_notification & SUB_1) {
				generatepacket(ERROR_PER, (uint32_t) (CAMERA & SUB_1), 4);
				handle_transmit(0);
			} else if (received_notification & SUB_2) {
				generatepacket(ERROR_PER, (uint32_t) (CAMERA & SUB_2), 4);
				handle_transmit(0);
			}
        }

        if (received_notification & WARNING & LOW_POWER) {
        	// Can't take photos in LOW POWER mode
        	// Tell ground station low power, no picture
        	// Call transmit
        	generatepacket(ERROR_LP, NULL, 0);
        	handle_transmit(0);
        }

        if (received_notification & REQUEST & GPS) {
			// Clear GPS sector
			flash_clear(FLASH_SECTOR_GPS);
			// Read GPS - writes to flash
        	bool gps_result = get_gps();

			// Send task notification to the OBC
			if(gps_result == true) {
				xTaskNotify(obc_notifications, INFO & GPS, eSetValueWithOverwrite);
			} else {
				xTaskNotify(obc_notifications, ERROR & GPS, eSetValueWithOverwrite);
				generatepacket(ERROR_PER, (uint32_t)(GPS), 4);
				handle_transmit(0);
			}
			// Transmit all telemetry data from flash
			int telem_length = sizeof(BatteryData) + sizeof(SensorsData) + 11;
			uint8_t telem_data[telem_length];
			if(!flash_read(telem_data, sizeof(BatteryData), FLASH_SECTOR_BATTERY, BATTERY_DATA_OFFSET)) {
				// Error when reading
				xTaskNotify(obc_notifications, ERROR & MEMORY, eSetValueWithOverwrite);
				for(int i = 0; i < sizeof(BatteryData); i++) {
					telem_data[i] = 0xFF;
				}
			}
			if(!flash_read(&telem_data[sizeof(BatteryData)], sizeof(SensorsData), FLASH_SECTOR_SENSORS, SENSOR_DATA_OFFSET)) {
				// Error when reading
				xTaskNotify(obc_notifications, ERROR & MEMORY, eSetValueWithOverwrite);
				for(int i = 0; i < sizeof(SensorsData); i++) {
					telem_data[i+sizeof(BatteryData)] = 0xFF;
				}
			}
			if(!flash_read(&telem_data[sizeof(BatteryData) + sizeof(SensorsData)], 11, FLASH_SECTOR_GPS, 0)) {
				// Error when reading
				xTaskNotify(obc_notifications, ERROR & MEMORY, eSetValueWithOverwrite);
				for(int i = 0; i < 11; i++) {
					telem_data[i+sizeof(BatteryData)+sizeof(SensorsData)] = 0xFF;
				}
			}
			generatepacket(TELEMETRY, telem_data, telem_length);
			handle_transmit(0);
        }
        received_notification = 0;
    }
}

/*
 * @brief The task will trigger whenever the semaphore is given by the ISR. This is the information from the CC1201. The information
 * enters a queue
 *
 * @parameters: void *pvParameters
 *
 *
 *
 * @return true  Data was successfully pushed to the queue.
 * @return false Failed to push to queue (e.g., queue full or NULL queue handle).
 *
 *
 *
 */

void Task_receiveLL(void *pvParameters)
{
	while (1)
	{
		if (xSemaphoreTake(myBinarySem01Handle, portMAX_DELAY) == pdTRUE)
		{
			// ISR gave semaphore

			ReadRegisterBurst(); // Runs the LL function after semaphore triggered
		}
	}
}

void handle_transmit(int acknowlegement)
{
	/* Handle transmitting packet and retransmission */
	if (communication_status)
	{
		transmit();
		if(!acknowlegement) {
			xTimerStart(retransmission_timer, 0); // Start retransmission timer
		}
	}
}

void vRetransmissionTimerCallback(TimerHandle_t xTimer)
{
	// Timer callback function for handling retransmission
	// Get number of times timer has expired (saved in timer ID)
	uint32_t expiry_count = (uint32_t)pvTimerGetTimerID(xTimer);
	expiry_count++;

	// Compare last received seq num
	if (sequenceNum == last_received_seq_num)
	{
		// Packet acknowledged yay!
		expiry_count = 0;
		vTimerSetTimerID(xTimer, (void *)expiry_count);
		xTimerStop(xTimer, 0);
		return;
	}
	// If not acked, check if we have more attempts or not
	if (expiry_count == MAX_TRANS_ATTEMPTS)
	{
		// Enter lost state
		communication_status = COMM_LOST;
		expiry_count = 0;
		vTimerSetTimerID(xTimer, (void *)expiry_count);
		xTimerStop(xTimer, 0);
		return;
	}
	vTimerSetTimerID(xTimer, (void *)expiry_count);
	handle_transmit(0);
}

void receive(void *vpParameters)
{
	/*
	 * Read from queue of bits:
	 *
	 * Length -> 1 byte
	 * Payload type -> 1 byte
	 * Data -> variable length (length - 2 [- 3])
	 * Offset -> 3 bytes
	 * Seq num -> 2 bytes
	 */
	// Read data from buffer
	uint8_t data_buffer[MAX_PACKET_SIZE];
	osStatus_t status = osMessageQueueGet(receivequeueHandle, &data_buffer, NULL, 0U); // wait for message
	if (status != osOK)
	{
		xTaskNotify(obc_notifications, ERROR & SUB_3, eSetValueWithOverwrite);
		return; // Error: we can't read the received data
	}
	// Received a packet so we should be in nominal state
	communication_status = COMM_NOMINAL;

	uint8_t packet_length = data_buffer[0];
	PayloadType packet_type = (PayloadType)data_buffer[1];

	switch (packet_type)
	{
	case PING:
		// Acknowledge ping
		generatepacket(PING, NULL, 0);
		handle_transmit(1);
		break;
	case REQ_NOMINAL:
		xTaskNotify(obc_notifications, REQUEST & NOMINAL, eSetValueWithOverwrite);
		uint16_t seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
		// Acknowledge nominal command
		generatepacket(ACK_REC_STATUS, seq_num, 2);
		handle_transmit(1);
		break;
	case REQ_LOW_POWER:
		xTaskNotify(obc_notifications, REQUEST & LOW_POWER, eSetValueWithOverwrite);
		uint16_t seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
		// Acknowledge low power command
		generatepacket(ACK_REC_STATUS, seq_num, 2);
		handle_transmit(1);
		break;
	case CAMERA_1_END:
		// request for image, notify OBC
		xTaskNotify(obc_notifications, REQUEST & CAMERA & SUB_1, eSetValueWithOverwrite);
		break;
	case CAMERA_2_END:
		// request for image, notify OBC
		xTaskNotify(obc_notifications, REQUEST & CAMERA & SUB_2, eSetValueWithOverwrite);
		break;
	case ACK_REC_CAMER:
		uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
		uint8_t camera = data_buffer[4];
		// Contains an offset
		int acked_offset = (data_buffer[5] << 16) |
								(data_buffer[6] << 8) |
								(data_buffer[7]);
		uint16_t seq_num = (data_buffer[8] << 8) | (data_buffer[9]);
		// set as acknowledged
		last_received_seq_num = acked_seq_num;
		// Stop timer because we're going to send another packet right away
		xTimerStop(retransmission_timer, 0);
		// Send next chunk
		uint16_t image_data_len;
		flash_read(&image_data_len, sizeof(uint16_t), FLASH_SECTOR_GPS, IMAGE_DATA_LEN_OFFSET);
		packageAndSendChunks(camera, FLASH_SECTOR_CAMERA, image_data_len, acked_offset);
		handle_transmit(0);
		break;
	case ACK_REC_TELEMETRY:
		// set as acknowledged
		uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
		last_received_seq_num = acked_seq_num;
		break;
	case ACK_REC_ERROR:
		// set as acknowledged
		uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
		last_received_seq_num = acked_seq_num;
		break;
	case ERROR_DUP:
		// last packet was received twice by GS
		uint16_t acked_seq_num = (data_buffer[2] << 8) | (data_buffer[3]);
		last_received_seq_num = acked_seq_num;
		break;
	default:
		// Error: should not be receiving other packet types
		// Notify OBC of packet error
		xTaskNotify(obc_notifications, ERROR & SUB_4, eSetValueWithOverwrite);
		break;
	}
}
