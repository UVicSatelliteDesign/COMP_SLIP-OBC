#include "ttc_interface.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include<stdlib.h>

#define MAX_PACKET_SIZE 256  //maximum packet size
#define PACKET_HEADER_SIZE 4 //identifier, power, status, length
#define PAYLOAD_SIZE 244     //max payload size
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

//very generic concept , will implement different ones to handle specific types
void prepare_package(uint8_t *payload, uint8_t payloadLen) {
    uint16_t totalLen = 1 + payloadLen + 2 + 4;
    uint8_t *txBuffer = (uint8_t *)malloc(totalLen);
    if (!txBuffer) return;

    uint16_t i = 0;

    //type
    txBuffer[i++] = type;

    //payload
    memcpy(&txBuffer[i], payload, payloadLen);
    i += payloadLen;

    //offset
    txBuffer[i++] = offset & 0xFF;
    txBuffer[i++] = (offset >> 8) & 0xFF;

    //sequence Number 
    txBuffer[i++] = sequenceNum & 0xFF;
    txBuffer[i++] = (sequenceNum >> 8) & 0xFF;
    txBuffer[i++] = (sequenceNum >> 16) & 0xFF;
    txBuffer[i++] = (sequenceNum >> 24) & 0xFF;

    //now send that buffer
   writeToTransmitBuffer(txBuffer, totalLen);

    //cleanup
    free(txBuffer);
}
