#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "microhttpd.h"
#include "prom.h"
#include "promhttp.h"
#include <protobuf-c/protobuf-c.h>

#include "power_metrics.h"
#include "../protobuffers/powermeasurements/powermeasurement.pb-c.h"

#define ENV_VAR_IP "DEVICE_UDP_IP" 
#define ENV_VAR_PORT "DEVICE_UDP_PORT"

#define IP_PROTOCOL 0
#define HTTP_DAEMON_PORT 8000

#define DEVICE_DATA_FETCH_INTERVAL_SECONDS 3
#define RECEIVE_BUFFER_SIZE 1024

typedef struct {
    char ip[50];
    uint16_t port;
} dev_udp_address_t;

static void fetch_data(void *args);
static void init_exporter();
static void handle_sigint(int signal);

static volatile sig_atomic_t running = 1;

struct MHD_Daemon *http_daemon;
int dev_communication_socket;

int main(int argc, char *argv[]) {
    dev_udp_address_t dev_udp_address;
    pthread_t dev_communication_thread_id;

    if (argc == 1) {
        // Determine IP and port via ENVs...
        if (!getenv(ENV_VAR_IP) || !getenv(ENV_VAR_PORT)) {
            printf("\nWrong usage - ENVs '%s' and '%s' must be set when no arguments are given!\n", ENV_VAR_IP, ENV_VAR_PORT);
            return EXIT_FAILURE;
        }
        strcpy(dev_udp_address.ip, getenv(ENV_VAR_IP));
        dev_udp_address.port = strtoul(getenv(ENV_VAR_PORT), NULL, 10);
    } else if (argc == 3) {
        // Determine IP and port via args ...
        strcpy(dev_udp_address.ip, argv[1]);
        dev_udp_address.port = strtoul(argv[2], NULL, 10);
    } else {
        printf("\nWrong usage - Execute %s <device_udp_ip> <device_udp_port> or just %s with ENVs '%s' and '%s' being set.\n",
               argv[0], ENV_VAR_IP, ENV_VAR_PORT);
        return EXIT_FAILURE;
    }

    struct sigaction handler;
    handler.sa_handler = handle_sigint;
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0) {
        printf("\nFailed to register the SIGINT handler: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    pthread_create(&dev_communication_thread_id, NULL, fetch_data, &dev_udp_address);

    init_exporter();
    while (running) {
        // Expose metrics ... Already done by the http daemon.
    }

    printf("\nShutting down ...\n");
    shutdown(dev_communication_socket, 0);
    close(dev_communication_socket);
    return EXIT_SUCCESS;
}

/**
 * Starts the communication with the monitored device via UDP in order to fetch
 * the data needed for updating the metrics.
 * 
 * The monitored device is expected to send power measurements serialized as
 * a corresponding protocolbuffer upon a UDP request sent by the exporter.
 * 
 * Illustration:
 *      exporter -- UDP (request) --> device 
 *        ^                              |
 *        |                              |
 *        -------- UDP (response) --------
 * 
 * @param args pointer to the argument, which must be of type dev_udp_address_t.
 */
static void fetch_data(void *args) {
    dev_udp_address_t *dev_addr = args;
    uint8_t rx_buffer[RECEIVE_BUFFER_SIZE];
    size_t rx_data_len;

    struct sockaddr_in dev_sockaddr;
    dev_sockaddr.sin_family = AF_INET;
    dev_sockaddr.sin_port = htons(dev_addr->port); 

    if (inet_pton(AF_INET, dev_addr->ip, &dev_sockaddr.sin_addr) <= 0) {
        printf("\nThe address %s is either invalid or not supported!\n", dev_addr->ip);
        exit(EXIT_FAILURE);
    }
    if ((dev_communication_socket = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)) < 0) {
        printf("\nFailed to create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (connect(dev_communication_socket, (struct sockaddr *)&dev_sockaddr, sizeof(dev_sockaddr)) < 0) {
        printf("\nConnection failed: %s\n", strerror(errno));
        shutdown(dev_communication_socket, 0);
        close(dev_communication_socket);
        exit(EXIT_FAILURE);
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 500000; // 500ms
    if (setsockopt(dev_communication_socket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) < 0) {
        perror("Failed to set the socket read timeout!\n");
        exit(EXIT_FAILURE);
    }
        
    printf("Starting to fetch data from monitored device via UDP (IP: %s | Port: %d)\n", dev_addr->ip, dev_addr->port);
    while (1) {
        send(dev_communication_socket, "HELLO", strlen("HELLO"), 0);

        rx_data_len = read(dev_communication_socket, rx_buffer, RECEIVE_BUFFER_SIZE);
        if (rx_data_len < 0) {
            printf("Failed to receive data from the monitored device: %s\n", strerror(errno));
        } else {
            PowerMeasurement *measurement = power_measurement__unpack(NULL, rx_data_len, rx_buffer);
            if (measurement == NULL) {
                printf("Failed to deserialize the received data!\n");
            } else {
                printf("\nTimestamp: %lu \nEnergy Consumption: %lf mAs \nCurrent: %f mA\n\n", 
                       measurement->timestamp, measurement->energy_consumption, measurement->current);

                power_measurement__free_unpacked(measurement, NULL);
                update_power_metrics(*measurement);
            }
        }

        sleep(DEVICE_DATA_FETCH_INTERVAL_SECONDS);
    }
}

/**
 * Initializes the exporter by setting up the default collector
 * registry, the power consumption related metrics and the HTTP daemon.
 */
static void init_exporter() {
    int err = prom_collector_registry_default_init();
    if (err) {
        printf("\nFailed to initialize the default collector registry!\n");
        exit(err);
    }
    printf("Successfully initialized the default collector registry.\n");

    init_power_metrics();
    promhttp_set_active_collector_registry(NULL);

    printf("Starting HTTP daemon on port %d in the background.\n", HTTP_DAEMON_PORT);
    http_daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTP_DAEMON_PORT, NULL, NULL);
    if (http_daemon == NULL) {
        printf("\nFailed to start the exporter HTTP daemon: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void handle_sigint(int signal) {
    prom_collector_registry_destroy(PROM_COLLECTOR_REGISTRY_DEFAULT);
    MHD_stop_daemon(http_daemon);
    running = 0;
}