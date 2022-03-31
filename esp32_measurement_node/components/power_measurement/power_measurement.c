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

static const char *TAG = "POWER_MEASUREMENT";

static ina3221_t dev = {
    .shunt = {100, 100, 50}, // shunt values are 100 mOhm for each channel
    .config.config_register = INA3221_DEFAULT_CONFIG,
    .mask.mask_register = INA3221_DEFAULT_MASK
};

float shunt_voltage;
float shunt_current2;
float shunt_current3;

uint64_t energy = 0;
uint64_t last_measurement = 0;

/*
 * Initializes the INA3221 power measurement sensor.
 */
void init_INA3221(void) {
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
 * Executes a power measurement for channel 2 and 3 and stores the current (mA)
 * in the variables shunt_current2 and shunt_current3 respectively.
 */
void exec_measurement(void) {
    ESP_ERROR_CHECK(ina3221_get_status(&dev)); // get mask

    ESP_ERROR_CHECK(ina3221_get_shunt_value(&dev, 1, &shunt_voltage, &shunt_current2));
    ESP_ERROR_CHECK(ina3221_get_shunt_value(&dev, 2, &shunt_voltage, &shunt_current3));
}

float get_shunt_current(void) {
    return shunt_current3;
}

/**
 * Initializes the INA3221 device and starts the power measurement flow.
 * 
 * This function should be handed to xTaskCreate() in order to start the power
 * measurements in a dedicated task.
 * 
 * @param task_params pointer to the task's parameters.
 */
void start_power_measurements(void *task_params) {
    init_INA3221();

    ESP_LOGI(TAG, "Starting the power measurements ...");
    last_measurement = esp_timer_get_time();

    while (1) {
        uint64_t timestamp_diff_microseconds;
        uint64_t new_measurement;

        exec_measurement();
        new_measurement = esp_timer_get_time();

        timestamp_diff_microseconds = new_measurement - last_measurement;
        energy += shunt_current3 * timestamp_diff_microseconds;
        
        last_measurement = new_measurement;
        vTaskDelay(pdMS_TO_TICKS(MEASUREMENT_INTERVAL_MS));
    }
}
