/*
 * bms.h
 *
 *  Created on: Feb 18, 2025
 *      Author: lelandsion
 */

#ifndef SRC_BMS_H_
#define SRC_BMS_H_

#include "stm32h7xx_hal.h"

typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float state_of_health;
} BatteryData;

void init_bms(void);
BatteryData get_battery_data(void);



#endif /* SRC_BMS_H_ */
