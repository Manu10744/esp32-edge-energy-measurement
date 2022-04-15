#ifndef __POWER_METRICS_H__
#define __POWER_METRICS_H__

#include "../protobuffers/powermeasurements/powermeasurement.pb-c.h"

void init_power_metrics(void);
void update_power_metrics(PowerMeasurement measurement);

#endif