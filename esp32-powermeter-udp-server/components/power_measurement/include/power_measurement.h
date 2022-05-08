#ifndef __POWER_MEASUREMENT_H__
#define __POWER_MEASUREMENT_H__

#include "../powermeasurement.pb-c.h"

PowerMeasurement get_measurement(int channel);
void start_power_measurements(void *task_param);

#endif
