#include "obc.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart3; // Use UART for debugging

void OBC_Task(void *argument) {
    BatteryData receivedData;
    char message[100]; // Buffer for UART messages

    while (1) {
        // Wait for data from BMS queue
        if (osMessageQueueGet(bmsQueue, &receivedData, NULL, osWaitForever) == osOK) {
            // Convert float values to strings and send via UART
            snprintf(message, sizeof(message),
                     "[OBC] Battery: V=%d.%02dV, I=%d.%02dA, SoC=%d.%02d%%, Power=%d.%02dW, Energy=%d.%02dWh\n",
                     (int)receivedData.voltage, (int)(receivedData.voltage * 100) % 100,
                     (int)receivedData.current, (int)(receivedData.current * 100) % 100,
                     (int)receivedData.state_of_charge, (int)(receivedData.state_of_charge * 100) % 100,
                     (int)receivedData.power_usage, (int)(receivedData.power_usage * 100) % 100,
                     (int)receivedData.total_energy_used, (int)(receivedData.total_energy_used * 100) % 100);

            HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
        }

        osDelay(5000); // Update every 5 seconds
    }
}
