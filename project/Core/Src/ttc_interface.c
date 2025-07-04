#include "ttc_interface.h"
#include "stm32h7xx_hal.h"

uint8_t gps_rx_buffer[]; // gps reception buffer
//////////////// section variables need definition
uint8_t NMEA_sentence_size; //  the length in bits of the gps data sentence
uint8_t interrupt_timeout_length; //  the time until the CPU unfreezes if data has not yet been received
////////////////
extern UART_HandleTypeDef huart3; // this is the gps reception pin handler


///////// get_gps() is waiting on a write to flash feature
void get_gps(){ // this is formatted for polling
    HAL_UART_Receive (&huart3, gps_rx_buffer, NMEA_sentence_size, interrupt_timeout_length);
}