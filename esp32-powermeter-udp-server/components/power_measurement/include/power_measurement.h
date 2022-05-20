#ifndef __POWER_MEASUREMENT_H__
#define __POWER_MEASUREMENT_H__

#include "../powermeasurement.pb-c.h"

PowerMeasurement get_measurement(uint8_t channel);
void start_power_measurements(void *task_param);
uint8_t get_amount_of_channels(void);

#endif
