#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define PORT CONFIG_PORT

static const char *TAG = "UDP_CLIENT";

void connect_to_server() {
    ESP_LOGI(TAG, "Connecting to UDP server on port %d ...", PORT);
}

void app_main(void) {
    esp_log_level_set(TAG, ESP_LOG_INFO);

    connect_to_server();
    setup_wifi();
}