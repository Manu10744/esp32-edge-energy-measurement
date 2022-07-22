#include <stdio.h>
#include <ina3221.h>
#include <string.h>
#include <math.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "power_measurement.h"

#define WARNING_CHANNEL 1 
#define WARNING_CURRENT (200.0)

#define I2C_PORT 1
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#define AMOUNT_OF_SENSORS CONFIG_AMOUNT_OF_INA3221_SENSORS
#define CHANNELS_PER_SENSOR 3
#define AMOUNT_OF_CHANNELS AMOUNT_OF_SENSORS * CHANNELS_PER_SENSOR
#define CONTINOUS_MEASUREMENT_MODE true
#define MEASUREMENT_INTERVAL_MS CONFIG_MEASUREMENT_INTERVAL

static void init_measurements();
static void init_ina3221(ina3221_t *dev, uint8_t i2c_addr);
static PowerMeasurement exec_measurement(uint8_t channel);
static ina3221_t get_ina3221_for_channel(uint8_t requested_channel);
static ina3221_channel_t get_channel(uint8_t requested_channel);

static const char *TAG = "POWER_MEASUREMENT";

static ina3221_t ina3221_vs = {
    .shunt = {100, 100, 100}, 
    .config.config_register = INA3221_DEFAULT_CONFIG,
    .mask.mask_register = INA3221_DEFAULT_MASK
};

static ina3221_t ina3221_gnd = {
    .shunt = {100, 100, 100},
    .config.config_register = INA3221_DEFAULT_CONFIG,
    .mask.mask_register = INA3221_DEFAULT_MASK
};

static ina3221_t ina3221_sensors[AMOUNT_OF_SENSORS];

PowerMeasurement measurements[AMOUNT_OF_CHANNELS];

/**
 * Initializes the INA3221 sensor(s) and starts measuring their channels.
 * The individual measurements per channel are stored in-memory.
 * 
 * This function should be handed to xTaskCreate() in order to start the power
 * measurements in a dedicated task.
 * 
 * @param task_param pointer to the task's parameter. In this case, no parameter
 *                   is used.
 */
void start_power_measurements(void *task_param) {
    ESP_ERROR_CHECK(i2cdev_init());

    ESP_LOGI(TAG, "Initializing INA3221 sensor (VS) ...");
    init_ina3221(&ina3221_vs, INA3221_I2C_ADDR_VS); 
    ESP_LOGI(TAG, "Initializing INA3221 sensor (GND) ...");
    init_ina3221(&ina3221_gnd, INA3221_I2C_ADDR_GND);

    ina3221_sensors[0] = ina3221_vs;
    ina3221_sensors[1] = ina3221_gnd;

    init_measurements();
    ESP_LOGI(TAG, "Initialization of the Power Measurements finished.");

    ESP_LOGI(TAG, "Starting the Power Measurements! (INA3221 sensors: %d | Available channels: %d)",
             AMOUNT_OF_SENSORS, AMOUNT_OF_CHANNELS);
    while (1) {
        for (uint8_t channel = 1; channel <= AMOUNT_OF_CHANNELS; channel++) {
            measurements[channel - 1] = exec_measurement(channel);
        }
        vTaskDelay(pdMS_TO_TICKS(MEASUREMENT_INTERVAL_MS));
    }
}

/**
 * Returns the most recent power measurement of the given INA3221 channel.
 * 
 * @param channel the desired INA3221 channel.
 * @return the most recent power measurement of the given channel.
 */
PowerMeasurement get_measurement(uint8_t channel) {
    if (channel > AMOUNT_OF_CHANNELS) {
        ESP_LOGE(TAG, "Cannot get measurement: Unknown channel %d", channel);
    }
    return measurements[channel - 1];
}

uint8_t get_amount_of_channels(void) {
    return AMOUNT_OF_CHANNELS;
}

/**
 * Executes a power measurement for the given INA3221 channel and returns the result.
 * 
 * @param channel the desired INA3221 channel that should be measured.
 * @return the new power measurement of the given channel.
 */
static PowerMeasurement exec_measurement(uint8_t channel) {
    ina3221_t sensor = get_ina3221_for_channel(channel);
    ina3221_channel_t sensor_channel = get_channel(channel);
    ESP_ERROR_CHECK(ina3221_get_status(&sensor)); 

    float shunt_current;
    float voltage;
    ESP_ERROR_CHECK(ina3221_get_shunt_value(&sensor, sensor_channel, &voltage, &shunt_current));

    PowerMeasurement prev_measurement = measurements[channel - 1];

    PowerMeasurement new_measurement = POWER_MEASUREMENT__INIT;
    new_measurement.timestamp = esp_timer_get_time();
    new_measurement.current = fabsf(shunt_current);

    float elapsed_seconds = (new_measurement.timestamp - prev_measurement.timestamp) * 1.0 / 1000000;
    new_measurement.energy_consumption = prev_measurement.energy_consumption + (fabsf(shunt_current) * elapsed_seconds);
    
    return new_measurement;
}

static void init_measurements() {
    for (uint8_t channel = 0; channel < AMOUNT_OF_CHANNELS; channel++) {
        PowerMeasurement init_measurement = POWER_MEASUREMENT__INIT;
        init_measurement.timestamp = esp_timer_get_time();

        measurements[channel] = init_measurement;
    }
}

/**
 * Initializes an INA3221 power measurement sensor that is connected via I2C.
 * This will enable all channels and use the continous mode for the measurements.
 * 
 * @param dev the pointer to the device descriptor of the INA3221 sensor.
 * @param i2c_addr the I2C address of the INA3221 sensor.
 */
static void init_ina3221(ina3221_t *dev, uint8_t i2c_addr) {
    ina3221_t ina3221_dev = *dev;
    memset(&ina3221_dev.i2c_dev, 0, sizeof(i2c_dev_t));

    ESP_ERROR_CHECK(ina3221_init_desc(dev, i2c_addr, I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN));
    ESP_ERROR_CHECK(ina3221_set_options(dev, CONTINOUS_MEASUREMENT_MODE, true, true));   // Mode selection, bus and shunt activated
    ESP_ERROR_CHECK(ina3221_enable_channel(dev, true, true, true));         // Enable all channels
    ESP_ERROR_CHECK(ina3221_set_average(dev, INA3221_AVG_1));                // 64 samples average
    ESP_ERROR_CHECK(ina3221_set_bus_conversion_time(dev, INA3221_CT_2116));   // 2ms by channel
    ESP_ERROR_CHECK(ina3221_set_shunt_conversion_time(dev, INA3221_CT_2116)); // 2ms by channel

    ESP_ERROR_CHECK(ina3221_set_warning_alert(dev, WARNING_CHANNEL - 1, WARNING_CURRENT)); // Set overcurrent security flag
}

/**
 * Returns the index of the INA3221 sensor that owns the given INA3221 channel.
 * For channels 1-3 the first INA3221 will be returned, the second INA3221 for channels 4-6, etc.
 * 
 * @param requested_channel the requested INA3221 channel.
 * @return the device descriptor of the INA3221 sensor that owns the requested channel.
 */
static ina3221_t get_ina3221_for_channel(uint8_t requested_channel) {
    uint8_t responsible_ina3221 = (requested_channel + (CHANNELS_PER_SENSOR-1)) / CHANNELS_PER_SENSOR;
    return ina3221_sensors[responsible_ina3221 - 1];
}

static ina3221_channel_t get_channel(uint8_t requested_channel) {
    uint8_t remainder = requested_channel % CHANNELS_PER_SENSOR;
    if (remainder == 0) {
        return INA3221_CHANNEL_3;
    } else if (remainder == 2) {
        return INA3221_CHANNEL_2;
    } else {
        return INA3221_CHANNEL_1;
    }
}