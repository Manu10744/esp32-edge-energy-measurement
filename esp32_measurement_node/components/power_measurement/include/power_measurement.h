#ifndef __POWER_MEASUREMENT_H__
#define __POWER_MEASUREMENT_H__

/**
 * Structure that contains the data related to a power consumption measurement
 * of one specific channel.
 * 
 *  - timestamp              timestamp of the measurement in microseconds.
 *  - energy_consumption     total power consumption at the time of this measurement.
 */
struct ina3221_measurement {
    uint64_t timestamp;
    uint64_t energy_consumption;
};

struct ina3221_measurement get_measurement(int channel);
void start_power_measurements(void *task_param);

#endif
