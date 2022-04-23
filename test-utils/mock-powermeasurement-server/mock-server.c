#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <protobuf-c/protobuf-c.h>

#include "../../protobuffers/powermeasurements/powermeasurement.pb-c.h"

#define PORT 10000
#define RECEIVE_BUFFER_SIZE 1024
#define SEND_INTERVAL_SECONDS 1
#define RANDOM_NUMBERS_MAX_VALUE 1000.0

static void start_server();
static void serve_client(void *client_sockaddr);
static void handle_sigint(int signal);

static volatile sig_atomic_t server_is_running = 1; 

int server_socket = 0;

int main() {
    // Set seed for random numbers
    srand((unsigned int)time(NULL));

    struct sigaction handler;
    handler.sa_handler = handle_sigint;
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0) {
        printf("Failed to register the SIGINT handler!\n");
        return EXIT_FAILURE;
    }   

    start_server();
    return EXIT_SUCCESS;
}

static void start_server() {
    struct sockaddr_in server_addr;
    char rx_buffer[RECEIVE_BUFFER_SIZE] = {0};

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&server_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    printf("Trying to setup server socket on Port %d ...\n", PORT);
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("Failed to create server socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("Successfully created the server socket. Now binding ...\n");
    int err = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err < 0) {
        printf("Failed to bind the server socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Successfully bound the listen socket! Starting to listen for incoming requests ...\n");
    while (server_is_running) {
        struct sockaddr_storage client_addr; // Large enough for both IPv4 or IPv6
        socklen_t client_socklen = sizeof(client_addr);

        // Ignore what we received and start sending mock measurements
        int rx_data_len = recvfrom(server_socket, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_socklen);
        if (rx_data_len < 0) {
            printf("Error during receiving on server socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        char *client_ip = inet_ntoa(((struct sockaddr_in *)&client_addr)->sin_addr);
        printf("Received %d bytes from new client (%s): %s\n", rx_data_len, client_ip, rx_buffer);

        pthread_t clientthread_id;
        pthread_create(&clientthread_id, NULL, serve_client, &client_addr);
    }  

    printf("Shutting down ...\n");
    shutdown(server_socket, 0);
    close(server_socket);
}

static PowerMeasurement create_mock_measurement(PowerMeasurement prev_mock_measurement) {
    double mock_consumption = (double)rand() / (double)(RAND_MAX / RANDOM_NUMBERS_MAX_VALUE);
    float mock_current = (float)rand() / (float)(RAND_MAX / RANDOM_NUMBERS_MAX_VALUE);

    PowerMeasurement mock_measurement = POWER_MEASUREMENT__INIT;
    mock_measurement.energy_consumption = prev_mock_measurement.energy_consumption + mock_consumption;
    mock_measurement.current = mock_current;
    return mock_measurement;
}

static void serve_client(void *sockaddr) {
    struct sockaddr_storage client_sockaddr = * (struct sockaddr_storage *)sockaddr;
    socklen_t client_socklen = sizeof(client_sockaddr);
    PowerMeasurement latest_mock_measurement = POWER_MEASUREMENT__INIT;

    void *tx_buffer;
    unsigned tx_data_len;

    while(1) {
        PowerMeasurement mock_measurement = create_mock_measurement(latest_mock_measurement);
        latest_mock_measurement = mock_measurement;

        tx_data_len = power_measurement__get_packed_size(&mock_measurement);
        tx_buffer = malloc(tx_data_len);
        power_measurement__pack(&mock_measurement, tx_buffer);

        int err = sendto(server_socket, tx_buffer, tx_data_len, 0, (struct sockaddr *)&client_sockaddr, client_socklen);
        if (err < 0) {
            printf("Error occured during sending on server socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        free(tx_buffer);
        sleep(SEND_INTERVAL_SECONDS);
    }    
}

static void handle_sigint(int signal) {
    server_is_running = 0;
    return;
}
