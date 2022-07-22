#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#ifdef CONFIG_USE_HOME_WIFI_SETTINGS
#define WIFI_SSID "FRITZ!Box 6591 Cable UK"
#define WIFI_PASSWORD "36982858034477962433"

#elif CONFIG_USE_INSTITUTE_WIFI_SETTINGS
#define WIFI_SSID "CAPS"
#define WIFI_PASSWORD "caps!schulz-wifi"
#endif

#define WIFI_MAX_RECONNECTS CONFIG_WIFI_MAX_RECONNECTS
#define WIFI_RECONNECT_WAIT_INTERVAL_SECONDS CONFIG_WIFI_RECONNECT_WAIT_INTERVAL_SECONDS

static const char *TAG = "WIFI";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

// Event Bits
#define WIFI_CONNECTED_BIT BIT0 // connected to the AP with an IP
#define WIFI_FAIL_BIT      BIT1 // failed to connect

static int reconnect_counter = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (reconnect_counter < WIFI_MAX_RECONNECTS) {
            ESP_LOGW(TAG, "WiFi connection could not be established or the existing connection got closed.");
            ESP_LOGI(TAG, "Waiting for %d seconds before initiating WiFi Reconnect...", WIFI_RECONNECT_WAIT_INTERVAL_SECONDS);
            vTaskDelay(pdMS_TO_TICKS(WIFI_RECONNECT_WAIT_INTERVAL_SECONDS * 1000));

            ESP_LOGI(TAG, "[%d/%d] Trying to reconnect to the WiFi AP ...", reconnect_counter, WIFI_MAX_RECONNECTS);
            esp_wifi_connect();
            reconnect_counter++;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connection to the WiFi AP was successful! Assigned IP: " IPSTR, IP2STR(&event->ip_info.ip));      
        
        reconnect_counter = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * Connects the ESP32 to a WiFi Access Point.
 * 
 * WiFi SSID and password are derived from the preconfigured WiFi settings
 * chosen in the project configuration.
 */
void connect_to_wifi(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Trying to setup WiFi connection for the configured Access Point ...");

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "Initialization of the WiFi Configuration finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to the Access Point (SSID: %s | password: %s)", WIFI_SSID, WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect the Access Point (SSID: %s | password: %s)", WIFI_SSID, WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "An unexpected event occured.");
    }
}
