#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <ssd1306.h>
#include <errno.h>
#include <string.h>

#include "wifi.h"
#include "power_measurement.h"

#define USE_DISPLAY CONFIG_USE_SSD1306_DISPLAY
#define DISPLAY_REFRESH_INTERVAL CONFIG_DISPLAY_REFRESH_INTERVAL

#define PORT CONFIG_PORT
#define RECEIVE_BUFFER_SIZE 16
#define SEND_INTERVAL_MS CONFIG_SEND_INTERVAL

static void serve_client(void *task_param);
static bool validate_request(char received_data[]);
static void start_udp_server(void *task_param);
static void show_measurements(void *task_param);

/**
 * Structure containing the relevant information about a client connection.
 * 
 *  - requested_channel    the INA3221 channel that was requested by the client.
 *  - sockaddr             socket address information.
 *  - socklen              length of the socket address information.
 */
struct client_info {
    int requested_channel;
    struct sockaddr_storage sockaddr;
    socklen_t socklen;
};

static const char *TAG = "UDP_SERVER";

int server_socket = 0;

void app_main(void) {
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Step 1: Establish a WiFi connection
    connect_to_wifi();

#ifdef CONFIG_USE_SSD1306_DISPLAY
    // Step 2: Activate SSD1306 display
    ESP_LOGI(TAG, "Using the SSD1306 display to visualize the measurements!");
    xTaskCreate(show_measurements, "display_power_measurements", configMINIMAL_STACK_SIZE * 8, NULL, 2, NULL);
#endif

    // Step 3: Start power measurement
    xTaskCreate(start_power_measurements, "ina3221_power_measurement", configMINIMAL_STACK_SIZE * 8, NULL, 2, NULL);

    // Step 4: Start UDP server and listen for connections
#ifdef CONFIG_USE_IPV4_STACK
    ESP_LOGI(TAG, "Using IPv4 stack for UDP server socket!");
    xTaskCreate(start_udp_server, "udp_server", 4096, (void*)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_USE_IPV6_STACK
    ESP_LOGI(TAG, "Using IPv6 stack for UDP server socket!");
    xTaskCreate(start_udp_server, "udp_server", 4096, (void*)AF_INET6, 5, NULL);
#endif 
}

/**
 * Starts a UDP server using the configured IP stack (IPv4 / IPv6) on the configured 
 * port. The server will restart itself on errors.
 * 
 * The server expects the client to send the INA3221 channel it wants to fetch
 * power measurements from. After receiving the requested channel it will start to
 * continuously send the power measurements.
 * 
 * This function should be handed to xTaskCreate() in order to start the UDP server 
 * in a dedicated task.
 * 
 * @param taskParams pointer to the task's parameter. This must be a pointer to either
 *                   AF_INET or AF_INET6.
 */
static void start_udp_server(void *task_param) {
    char rx_buffer[RECEIVE_BUFFER_SIZE] = {0};
    char addr_str[128];
    int addr_family = (int)task_param;
    int ip_protocol = 0;
    struct sockaddr_in6 server_addr;

    ESP_LOGI(TAG, "Trying to setup a UDP server on port %d ...", PORT);
    while (1) {
        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&server_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        } else if (addr_family == AF_INET6) {
            bzero(&server_addr.sin6_addr.un, sizeof(server_addr.sin6_addr.un));
            server_addr.sin6_family = AF_INET6;
            server_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        server_socket = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (server_socket < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Sucessfully created the UDP socket!");

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it must be disabled
            // if both protocols are used at the same time (used in CI)
            int opt = 1;
            setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int err = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Unable to bind socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket successfully bound to port %d!", PORT);

        ESP_LOGI(TAG, "Starting to listen for incoming connections ...");
        while (1) {
            struct sockaddr_storage client_addr; // Large enough for both IPv4 or IPv6
            socklen_t client_socklen = sizeof(client_addr);

            int rx_data_len = recvfrom(server_socket, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_socklen);
            if (rx_data_len < 0) {
                // Error occurred during receiving
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            
            if (client_addr.ss_family == PF_INET) {
                inet_ntoa_r(((struct sockaddr_in *)&client_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            } else if (client_addr.ss_family == PF_INET6) {
                inet6_ntoa_r(((struct sockaddr_in6 *)&client_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
            }

            rx_buffer[rx_data_len] = '\0';
            ESP_LOGI(TAG, "Received %d bytes from %s: %s", rx_data_len, addr_str, rx_buffer);

            if (validate_request(rx_buffer)) {
                int requested_channel = strtol(rx_buffer, NULL, 10);

                static struct client_info new_client;
                new_client.requested_channel = requested_channel;
                new_client.sockaddr = client_addr;
                new_client.socklen = client_socklen;

                xTaskCreate(serve_client, "serve_client", configMINIMAL_STACK_SIZE * 8, &new_client, 2, NULL);
            }
        }

        if (server_socket != -1) {
            ESP_LOGI(TAG, "Shutting down socket and restarting...");
            shutdown(server_socket, 0);
            close(server_socket);
        }
    }

    // Critical error occured
    vTaskDelete(NULL);
}

/**
 * Handles the serialized transmission of the power measurements retrieved
 * from the requested INA3221 channel to a specific client.
 * 
 * This function should be handed to xTaskCreate() in order to handle the transmission
 * of power measurements to the client in a dedicated task.
 * 
 * @param task_param pointer to the task's parameter. This must be a pointer to the 
 *                   structure (struct client_info) representing the information 
 *                   about the client being served.
 */
static void serve_client(void *task_param) {
    struct client_info client = * (struct client_info *)task_param;
    void *tx_buffer;
    unsigned tx_data_len;

    ESP_LOGI(TAG, "Starting to serve client requesting channel %d", client.requested_channel);
    while (1) {
        PowerMeasurement measurement = get_measurement(client.requested_channel);
        tx_data_len = power_measurement__get_packed_size(&measurement);
        tx_buffer = malloc(tx_data_len);
        power_measurement__pack(&measurement, tx_buffer);

        int err = sendto(server_socket, tx_buffer, tx_data_len, 0, (struct sockaddr *)&client.sockaddr, client.socklen);
        free(tx_buffer);

        if (err < 0) {
            if (errno == 118) {
                ESP_LOGE(TAG, "Error occured during sending data to client requesting channel %d: WiFi Connection is absent.", client.requested_channel);
            } else {
                ESP_LOGE(TAG, "Error occurred during sending data to client requesting channel %d: %s (errno %d)", client.requested_channel, strerror(errno), errno);
                break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(SEND_INTERVAL_MS));
    }

    ESP_LOGI(TAG, "Deleting task of client that's subscripted to channel %d", client.requested_channel);
    vTaskDelete(NULL);
}

/**
 * Validates a power measurement subscription request received from a client UDP 
 * message. 
 * 
 * In order to be valid, the received data must be a string representing a valid
 * INA3221 channel - meaning that it must be successfully converted to a positive integer
 * representing a valid INA3221 channel.
 * 
 * @param received_data the buffer containing the received data.
 * @return true if the request is valid, otherwise false.
 */
static bool validate_request(char received_data[]) {
    errno = 0;

    char *endptr = NULL; 
    long requested_channel = strtol(received_data, &endptr, 10);

    uint8_t available_channels = get_amount_of_channels();
    if (received_data == endptr) {
        ESP_LOGW(TAG, "Client subscription request for channel '%ld' is invalid - no channel ID is present in the request!",
                requested_channel);
        return false;
    } else if (errno == 0 && received_data && *endptr != 0) {
        ESP_LOGW(TAG, "Client subscription request for channel '%ld' is invalid - channel ID must be purely numeric!",
                requested_channel);
        return false;
    } else if (requested_channel < 1 || requested_channel > available_channels) {
        ESP_LOGW(TAG, "Client subscription request for channel '%ld' is invalid - channel ID must be inside interval of 1 <= requested channel <= %d", 
                requested_channel, available_channels);
        return false;
    } else if (errno != 0 && requested_channel == 0) {
        ESP_LOGW(TAG, "Client subscription request for channel '%ld' is invalid - unexpected error occurred!", 
                requested_channel);
        return false;
    }

    return errno == 0 && received_data && !*endptr;
}


/**
 * Displays the power measurements of the individual INA3221 channels on the SSD1306 display.
 * Standard pins are assumed to be used for the connection of the display via I2C.
 * 
 * This function should be handed to xTaskCreate() in order to start operating the
 * display in a dedicated task.
 * 
 * @param task_param pointer to the task's parameter. In this case, no parameter is used.
 */
static void show_measurements(void *task_param) {
    char buffer[200];

    ESP_LOGI(TAG, "Initializing SSD1306 Display ...");
    ssd1306_128x64_i2c_init();
    ssd1306_128x64_init();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_clearScreen();
    ESP_LOGI(TAG, "Initialization of the SSD1306 Display finished.");

    sprintf(buffer, "Hello :)");
    ssd1306_printFixedN(0, 0, buffer, STYLE_NORMAL, 2);

    uint8_t available_channels = get_amount_of_channels();
    while (1) {
        for (uint8_t channel = 1; channel <= available_channels; channel++) {
            PowerMeasurement curr_measurement = get_measurement(channel);

            ssd1306_clearScreen();

            sprintf(buffer, "Channel %d", channel);
            ssd1306_printFixedN(0, 0, buffer, STYLE_NORMAL, 0);
            
            sprintf(buffer, "%0.01f mAs", curr_measurement.energy_consumption);
            ssd1306_printFixedN(0, 20, buffer, STYLE_NORMAL, 0);
            sprintf(buffer, "%.01f mA", curr_measurement.current);
            ssd1306_printFixedN(0, 40, buffer, STYLE_NORMAL, 0);

            vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_INTERVAL));
        }
    }
}