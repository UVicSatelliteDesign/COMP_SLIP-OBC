#ifndef TTC_INTERFACE_H
#define TTC_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "cmsis_os2.h"
#include <semphr.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"

typedef uint8_t FIFOsize[128]; // new typedef used for defining FIFO size as mentioned in the CC1201 datasheet

extern osMessageQueueId_t receivequeueHandle; // the message queue used for receiving info from the trasnciever
extern osSemaphoreId_t myBinarySem01Handle;   // used to protect the dirty bit. it is a sempahore
void writeToDataBuffer(uint8_t *buffer, uint8_t *data, int length);
void generatepacket(uint8_t type, uint8_t *payload, uint8_t payloadLen);
void packageAndSendChunks(uint8_t type, uint8_t *payload, uint16_t fullPayloadLen, uint32_t sequenceNum);

void transmit();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

bool ReadRegisterBurst();

#endif
