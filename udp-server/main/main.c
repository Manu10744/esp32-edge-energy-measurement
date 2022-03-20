#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "lwip/sockets.h"

#include "wifi.h"

#define PORT CONFIG_PORT

static const char *TAG = "UDP_SERVER";

/**
 * Starts a UDP server using the configured IP stack (IPv4 / IPv6) on the configured 
 * port. Server will restart itself on errors.
 * 
 * This function should be handed to xTaskCreate() in order to start the UDP server 
 * in a dedicated task.
 * 
 * @param taskParams pointer to the task's parameters.
 */
void setup_udp_server(void *task_params) {
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)task_params;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    ESP_LOGI(TAG, "Trying to setup a UDP server on port %d ...", PORT);
    while (1) {
        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        } else if (addr_family == AF_INET6) {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Sucessfully created the UDP socket!");

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Unable to bind socket: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket successfully bound to port %d", PORT);

        while (1) {
            ESP_LOGI(TAG, "Waiting for data ...");
        }
    }

    vTaskDelete(NULL);
}



void app_main(void) {
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Establish a WiFi connection
    connect_to_wifi();

    // Start UDP server and listen for connections
    /*
#ifdef CONFIG_USE_IPV4_STACK
    ESP_LOGI(TAG, "Using IPv4 stack!");
    xTaskCreate(setup_udp_server, "udp_server", 4096, (void*)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_USE_IPV6_STACK
    ESP_LOGI(TAG, "Using IPv6 stack!");
    xTaskCreate(setup_udp_server, "udp_server", 4096, (void*)AF_INET6, 5, NULL);
#endif */
}