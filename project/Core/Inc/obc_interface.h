#ifndef SRC_OBC_INTERFACE_H_
#define SRC_OBC_INTERFACE_H_

#include "stm32h7xx_hal.h"

// Battery data structure
typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float power_usage;
    float total_energy_used;
    float estimated_life;
} BatteryData;

// Function prototypes
void init_bms(void);
BatteryData get_battery_data(float dt);

#endif /* SRC_OBC_INTERFACE_H_ */
