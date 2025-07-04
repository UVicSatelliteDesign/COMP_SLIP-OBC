#include "ttc_interface.h"
#include "flash_interface.h"
#include "stm32h7xx_hal.h"

uint8_t gps_rx_buffer[]; // gps reception buffer
//////////////// section variables need definition
uint8_t NMEA_sentence_size; //  the length in bits of the gps data sentence
uint8_t interrupt_timeout_length; //  the time until the CPU unfreezes if data has not yet been received
uint16_t GPS_FLASH_ADDRESS; // not actual address, change to real address
////////////////
extern UART_HandleTypeDef huart3; // this is the gps reception pin handler


/*
get_gps() polls gps data into a buffer and writes the buffer to flash.
This function returns true if successful, and returns false otherwise*/
bool get_gps(){ // this is formatted for polling
    HAL_UART_Receive (&huart3, gps_rx_buffer, NMEA_sentence_size, interrupt_timeout_length);
    if(flash_write(*gps_rx_buffer, NMEA_sentence_size, GPS_FLASH_ADDRESS) == true){
        return true;
    }
    return false;
}