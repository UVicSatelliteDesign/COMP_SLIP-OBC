#ifndef OBC_H
#define OBC_H

#include "stm32h7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#define REQUEST_CAMERA 0x01
#define CAMERA_READY 0x02

typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float state_of_health;
} BatteryData;

void init_bms(void);
BatteryData get_battery_data(void);
void obc_notifications(void *vpParameters);

#endif // OBC_H
