#include <stdio.h>
#include "prom.h"
#include "metric_power_consumption.h"

#define METRIC_NAME_TOTAL_CONSUMPTION "powerexporter_power_consumption_total_mAs"
#define METRIC_DESC_TOTAL_CONSUMPTION "The total power consumption."

#define METRIC_NAME_CONSUMPTION "powerexporter_power_consumption_mAs"
#define METRIC_DESC_CONSUMPTION "The amount of power consumed since the previous measurement was executed."

prom_counter_t *total_consumption;
prom_gauge_t *consumption;

void update_metrics() {
    // TODO: Poll UDP client for data and update metrics
}

void init_power_metrics(void) {
    printf("Creating and registering the power-related metrics ...\n");
    total_consumption = prom_collector_registry_must_register_metric(prom_counter_new(METRIC_NAME_TOTAL_CONSUMPTION, METRIC_DESC_TOTAL_CONSUMPTION, 0, NULL));
    consumption = prom_collector_registry_must_register_metric(prom_gauge_new(METRIC_NAME_CONSUMPTION, METRIC_DESC_CONSUMPTION, 0, NULL));
}