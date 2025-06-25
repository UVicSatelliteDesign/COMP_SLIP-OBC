#include "ttc_interface.h"
#include "stm32h7xx_hal.h"

uint8_t gps_rx_buffer[];
//////////////// section variables need definition
uint8_t NMEA_sentence_size; // NMEA sentence bit-length
uint8_t interrupt_timeout_length; // time until the CPU unfreezes if data has not yet been received
////////////////
extern UART_HandleTypeDef huart3;

void get_gps(){ // polling format
    HAL_UART_Receive (&huart3, gps_rx_buffer, NMEA_sentence_size, interrupt_timeout_length);
}