#ifndef SRC_OBC_H_
#define SRC_OBC_H_

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


#endif /* SRC_OBC_H_ */
