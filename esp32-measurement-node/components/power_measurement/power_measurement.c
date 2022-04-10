#include <stdio.h>
#include <ina3221.h>
#include <string.h>
#include "esp_timer.h"
#include "esp_log.h"

#include "power_measurement.h"

#define WARNING_CHANNEL 1 
#define WARNING_CURRENT (200.0)

#define I2C_PORT 1
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#define CONTINUOUS_MODE true
#define MEASUREMENT_INTERVAL_MS 10
#define AMOUNT_OF_CHANNELS CONFIG_AMOUNT_OF_CHANNELS

static const char *TAG = "POWER_MEASUREMENT";

static ina3221_t dev = {
    .shunt = {100, 100, 50}, // shunt values are 100 mOhm for each channel
    .config.config_register = INA3221_DEFAULT_CONFIG,
    .mask.mask_register = INA3221_DEFAULT_MASK
};

PowerMeasurement measurements[AMOUNT_OF_CHANNELS];

/*
 * Initializes the INA3221 power measurement sensor.
 */
static void init_ina3221(void) {
    ESP_LOGI(TAG, "Initializing INA3221 ...");
    memset(&dev.i2c_dev, 0, sizeof(i2c_dev_t));

    ESP_ERROR_CHECK(i2cdev_init());
    ESP_ERROR_CHECK(ina3221_init_desc(&dev, INA3221_I2C_ADDR_VS, I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN));

    ina3221_set_options(&dev, CONTINUOUS_MODE, true, true);   // Mode selection, bus and shunt activated
    ESP_ERROR_CHECK(ina3221_set_options(&dev, CONTINUOUS_MODE, true, true));   // Mode selection, bus and shunt activated
    ESP_ERROR_CHECK(ina3221_enable_channel(&dev, false, true, true));         // Enable all channels
    ESP_ERROR_CHECK(ina3221_set_average(&dev, INA3221_AVG_1));                // 64 samples average
    ESP_ERROR_CHECK(ina3221_set_bus_conversion_time(&dev, INA3221_CT_2116));   // 2ms by channel
    ESP_ERROR_CHECK(ina3221_set_shunt_conversion_time(&dev, INA3221_CT_2116)); // 2ms by channel

    ESP_ERROR_CHECK(ina3221_set_warning_alert(&dev, WARNING_CHANNEL - 1, WARNING_CURRENT)); // Set overcurrent security flag
}

/**
 * Executes a power measurement for the given INA3221 channel and returns
 * the structure representing the new measurement.
 * 
 * @param channel index of the channel that is measured.
 * @return the new power measurement of the given channel.
 */
static PowerMeasurement exec_measurement(int channel) {
    ESP_ERROR_CHECK(ina3221_get_status(&dev)); // get mask

    PowerMeasurement prev_measurement = measurements[channel];
    PowerMeasurement new_measurement = POWER_MEASUREMENT__INIT;
    float shunt_current;
    float voltage;

    ESP_ERROR_CHECK(ina3221_get_shunt_value(&dev, channel, &voltage, &shunt_current));
    new_measurement.timestamp = esp_timer_get_time();
    new_measurement.current = shunt_current;

    uint64_t elapsed_microseconds = new_measurement.timestamp - prev_measurement.timestamp;
    new_measurement.energy_consumption = prev_measurement.energy_consumption + (shunt_current * elapsed_microseconds);
    
    return new_measurement;
}

/**
 * Returns the most recent power measurement of the given INA3221 channel.
 * 
 * @param channel index of the channel that is measured.
 * @return the most recent power measurement of the given channel.
 */
PowerMeasurement get_measurement(int channel) {
    if (channel >= AMOUNT_OF_CHANNELS) {
        ESP_LOGE(TAG, "Unknown channel: %d", channel);
    }
    return measurements[channel];
}

/**
 * Initializes the measurements for each INA3221 channel.
 */
static void init_measurements() {
    for (int channel = 0; channel < AMOUNT_OF_CHANNELS; channel++) {
        PowerMeasurement init_measurement = POWER_MEASUREMENT__INIT;
        init_measurement.timestamp = esp_timer_get_time();

        measurements[channel] = init_measurement;
    }
}

/**
 * Initializes the INA3221 device and starts the power measurement flow.
 * 
 * This function should be handed to xTaskCreate() in order to start the power
 * measurements in a dedicated task.
 * 
 * @param task_param pointer to the task's parameters. In this case, no parameter
 *                   is required.
 */
void start_power_measurements(void *task_param) {
    init_ina3221();
    init_measurements();

    ESP_LOGI(TAG, "Starting the power measurements ...");
    while (1) {
        for (int channel = 0; channel < AMOUNT_OF_CHANNELS; channel++) {
            measurements[channel] = exec_measurement(channel);
        }
        vTaskDelay(pdMS_TO_TICKS(MEASUREMENT_INTERVAL_MS));
    }
}
