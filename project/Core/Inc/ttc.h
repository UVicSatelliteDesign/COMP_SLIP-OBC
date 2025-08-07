#ifndef TTC_H
#define TTC_H
#include "ttc_interface.h"

#define COMM_NOMINAL 1
#define COMM_LOST 0

/* Timer handle, created in main */
extern TimerHandle_t retransmission_timer; // Timer handle, created in main

/**
 * ttc_notifications:
 * Task that receives notifications from OBC tasks and handles them; 
 * transmits error, telemetry, and image data packets.
 */

/**
 * handle_transmit:
 * Handles transmitting a packet once it has been generated and
 * starts the retransmission timer if applicable. 
 * 
 * Parameters:
 * acknowledgement: true (1) if packet is an acknowledgement, false (0) otherwise
 */
void handle_transmit(int acknowlegement);

/**
 * vRetransmissionTimerCallback:
 * Callback function for the retransmission timer. Checks if packet 
 * has been acknowledged and retransmits if not until max
 * retransmission attempts have occured, then puts TTC in lost state.
 * 
 * Parameters:
 * xTimer: Timer handle
 */
void vRetransmissionTimerCallback( TimerHandle_t xTimer );

/**
 * receive:
 * Task which parses data received from the TTC and starts
 * appropriate actions.
 */

#endif // TTC_H
