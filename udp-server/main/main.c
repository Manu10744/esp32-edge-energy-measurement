#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define PORT CONFIG_PORT

static const char *TAG = "UDP_SERVER";

void setup_udp_server() {
    ESP_LOGI(TAG, "Setting up UDP server on Port %s ...", PORT);
}

void app_main(void) {
    esp_log_level_set(TAG, ESP_LOG_INFO);

    setup_udp_server();
}