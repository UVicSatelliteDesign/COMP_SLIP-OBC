#ifndef TTC_H
#define TTC_H
#include "ttc_interface.h"

#define COMM_NOMINAL 1
#define COMM_LOST 0

extern TimerHandle_t retransmission_timer; // Timer handle, created in main

void ttc_notifications(void *vpParameters);

void vRetransmissionTimerCallback( TimerHandle_t xTimer );

void receive();

#endif // TTC_H
