#include "ttc_interface.h"
#include "flash_interface.h"
#include "stm32h7xx_hal.h"

uint8_t gps_rx_buffer[]; // gps reception buffer
//////////////// section variables need definition
uint8_t NMEA_sentence_size; //  the length in bits of the gps data sentence
uint8_t interrupt_timeout_length; //  the time until the CPU unfreezes if data has not yet been received
uint16_t GPS_FLASH_ADDRESS; // to be set before implementation
////////////////
extern UART_HandleTypeDef huart3; // GPS UART handle definition


/*
get_gps:
    polls an NMEA $GPRMC gps sentence from the gps unit and stores this data to flash
Parameters:
    void
Returns:
    returns a true boolean if successful, and a false boolean otherwise

*/
bool get_gps(){ // this is formatted for polling
    HAL_UART_Receive (&huart3, gps_rx_buffer, NMEA_sentence_size, interrupt_timeout_length);
    if(flash_write(*gps_rx_buffer, NMEA_sentence_size, GPS_FLASH_ADDRESS) == true){
        return true;
    }
    return false;
}