#include "obc.h"
#include "cmsis_os.h"

void obc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    while (1) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & REQUEST_CAMERA) {
            take_snapshot(/* TODO: Create and add camera configurations */);

            // TODO: Send confirmation notification
        }
    }
}

data_task(void *vpParamters) {
	float voltage;
	float current;
	float temperature;
	// TODO: Add other data points
	while (1) {
		voltage = read_battery_voltage();
		current = read_battery_current();
		temperature = read_battery_temperature();
		// TODO: Read other sensor data

		// TODO: Send data to TTC

		// TODO: Store data in memory
	}
}
