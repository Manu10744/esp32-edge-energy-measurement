#include <stdio.h>
#include "prom.h"
#include "power_metrics.h"

#define METRIC_NAME_TOTAL_CONSUMPTION "powerexporter_power_consumption_ampere_seconds_total"
#define METRIC_DESC_TOTAL_CONSUMPTION "The total power consumption given in ampere-seconds."

#define METRIC_NAME_CURRENT "powerexporter_current_ampere"
#define METRIC_DESC_CURRENT "The electric current given in ampere."

/** 
 * Structure wrapping around a prom_counter_t to keep track of its value.
 * 
 *    - counter   pointer to the prom_counter_t.
 *    - value     the current value of the counter.
 */
typedef struct {
    prom_counter_t *counter;
    double value;
} prom_counter_wrapper_t;

static void prom_counter_increment_to(prom_counter_wrapper_t *counter_wrapper, double new_value);

prom_gauge_t *current_gauge;
prom_counter_wrapper_t consumption_counter_wrapper;

void init_power_metrics(void) {
    printf("Creating and registering the power-related metrics ...\n");
    consumption_counter_wrapper.counter = prom_collector_registry_must_register_metric(prom_counter_new(METRIC_NAME_TOTAL_CONSUMPTION, METRIC_DESC_TOTAL_CONSUMPTION, 0, NULL));
    consumption_counter_wrapper.value = 0;
    current_gauge = prom_collector_registry_must_register_metric(prom_gauge_new(METRIC_NAME_CURRENT, METRIC_DESC_CURRENT, 0, NULL));
}

void update_power_metrics(PowerMeasurement measurement) {
    prom_gauge_set(current_gauge, measurement.current, NULL);
    prom_counter_increment_to(&consumption_counter_wrapper, (double) measurement.energy_consumption);
}

/**
 * Increments the prometheus counter in counter_wrapper up to the value of new_value.
 * 
 * Note: This is a workaround of the prometheus-client-c library, which only provides 
 * the possibility to increment by one or add a fixed value to the counter.
 * 
 * @param counter_wrapper pointer to the wrapper of the counter that is incremented.
 * @param new_value new value for the counter - must be greater or equal than
 *                  the current counter value.
 */ 
static void prom_counter_increment_to(prom_counter_wrapper_t *counter_wrapper, double new_value) {
    if (new_value < 0 || new_value < counter_wrapper->value) {
        printf("Illegal Operation - Can't increment counter to a lower value!\n");
        exit(EXIT_FAILURE);
    }

    double diff = new_value - counter_wrapper->value;
    prom_counter_add(counter_wrapper->counter, diff, NULL);
    
    counter_wrapper->value = new_value;
}



