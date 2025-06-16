#include "ttc_interface.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include<stdlib.h>

#define MAX_PACKET_SIZE 32
#define HEADER_SIZE 7
#define MAX_PAYLOAD_PER_PACKET (MAX_PACKET_SIZE - HEADER_SIZE)
#define SHARED_MEMORY_ADDRESS ((uint8_t*)0x80000000)

typedef enum {
    PING = 0b00000000,
    NOMINAL = 0b00000001,
    LOW_POWER = 0b00000010,
    TELEMETRY = 0b00000011,
    CAMERA_1_END = 0b00000100,
    CAMERA_1_MF = 0b00000101,
    CAMERA_2_END = 0b00000110,
    CAMERA_2_MF = 0b00000111,
    REQ_RETRANSMISSION = 0b00001000,
    ERROR_CRC = 0b00001001,
    ERROR_DUP = 0b00001010,
    ERROR_LP = 0b00001011, 
    ACK_REC_CAMER = 0b00001100,
    ACK_REC_TELEMETRY = 0b00001101,
    ACK_REC_STATUS = 0b00001110,
    ACK_REC_ERROR = 0b00001111,
} PayloadType;

void generatepacket(uint8_t type, uint8_t *payload, uint8_t payloadLen) {
    // Send type
    writeToTransmitBuffer(&type, 1);

    // Send payload
    writeToTransmitBuffer(payload, payloadLen);

    // Send offset (little-endian because of system memory layout)
    writeToTransmitBuffer((uint8_t *)&offset, 2);

    // Send sequence number (little-endian)
    writeToTransmitBuffer((uint8_t *)&sequenceNum, 4);
}

void packageAndSendChunks(uint8_t type, uint8_t *payload, uint16_t fullPayloadLen, uint32_t sequenceNum) {
    uint16_t offset = 0;

    while (offset < fullPayloadLen) {
        // Determine the size of this chunk's payload
        uint8_t chunkLen = (fullPayloadLen - offset > MAX_PAYLOAD_PER_PACKET)
                           ? MAX_PAYLOAD_PER_PACKET
                           : (fullPayloadLen - offset);

        // Send type
        writeToTransmitBuffer(&type, 1);

        // Send payload chunk
        writeToTransmitBuffer(&payload[offset], chunkLen);

        // Send offset (little-endian)
        writeToTransmitBuffer((uint8_t *)&offset, 2);

        // Send sequence number (little-endian)
        writeToTransmitBuffer((uint8_t *)&sequenceNum, 4);

        // Move to next chunk
        offset += chunkLen;
    }
}
