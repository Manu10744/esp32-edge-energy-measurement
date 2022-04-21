#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include <protobuf-c/protobuf-c.h>

#include "udp_client.h"
#include "../protobuffers/powermeasurements/powermeasurement.pb-c.h"

#define IP_PROTOCOL 0
#define RECEIVE_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 16

#define LISTEN_SOCKET_PORT 5000

static void handle_sigint(int signal);
static void shutdown_sock(int sock);
static void listen_for_requests();

static volatile sig_atomic_t listen_for_data = 1; 

PowerMeasurement latest_measurement;

int main(int argc, char *argv[]) {
    char server_ip[50];
    uint16_t port;
    uint8_t requested_channel;
    pthread_t listen_thread_id;

    if (argc != 4) {
        printf("\nUsage: %s <server_ip> <server_port> <channel>\n", argv[0]);
        return EXIT_FAILURE;
    }
    strcpy(server_ip, argv[1]);
    port = strtoul(argv[2], NULL, 10);
    requested_channel = strtoul(argv[3], NULL, 10);

    struct sigaction handler;
    handler.sa_handler = handle_sigint;
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0) {
        printf("Failed to register the SIGINT handler!\n");
        return EXIT_FAILURE;
    }   

    pthread_create(&listen_thread_id, NULL, listen_for_requests, NULL);
    fetch_power_measurements(server_ip, port, requested_channel);
}

/**
 * Initiates the UDP communication with the server sending the power measurements.
 * 
 * The requested INA3221 channel is sent to the server, which is expected to continously 
 * send the corresponding power measurements once the request was received.
 * 
 * @param server_ip the IP of the target server.
 * @param port the port of the target server.
 * @param requested_channel the channel to request power measurements from.
 */
void fetch_power_measurements(char server_ip[], uint16_t port, uint8_t requested_channel) {
    int client_socket = 0;
    int rx_data_len;
    char rx_buffer[RECEIVE_BUFFER_SIZE] = {0};
    char tx_buffer[SEND_BUFFER_SIZE] = {0};
    struct sockaddr_in serv_addr;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);   

    printf("Starting communication with server on IP: %s | Port: %d and requesting data for channel %d!\n", server_ip, port, requested_channel);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nThe address %s is either invalid or not supported!\n", server_ip);
        exit(EXIT_FAILURE);
    }
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)) < 0) {
        printf("\nCould not create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed: %s\n", strerror(errno));
        shutdown_sock(client_socket);
        exit(EXIT_FAILURE);
    }
    
    int tx_data_len = sprintf(tx_buffer, "%u", requested_channel);
    send(client_socket, tx_buffer, tx_data_len + 1, 0);

    while (listen_for_data) {
        rx_data_len = read(client_socket, rx_buffer, RECEIVE_BUFFER_SIZE);
        if (rx_data_len < 0) {
            printf("Error during receiving: %s\n", strerror(errno));
            break;
        }
        printf("Received %d bytes from %s", rx_data_len, server_ip);

        PowerMeasurement *measurement = power_measurement__unpack(NULL, rx_data_len, rx_buffer);
        if (measurement == NULL) {
            printf("Failed to deserialize the received data!\n");
            exit(EXIT_FAILURE);
        }
        latest_measurement = *measurement;
        power_measurement__free_unpacked(measurement, NULL);

        printf("\nTimestamp: %lu \nEnergy Consumption: %lf mAs \nCurrent: %f mA\n\n", 
            latest_measurement.timestamp, latest_measurement.energy_consumption, latest_measurement.current);
    }

    shutdown_sock(client_socket);
}

/**
 * Starts a listen socket that waits for incoming UDP messages and responds
 * with the latest power measurement to the sender of the request.
 */
static void listen_for_requests() {
    int listen_socket = 0;
    struct sockaddr_in server_addr, client_addr;
    char rx_buffer[RECEIVE_BUFFER_SIZE] = {0};

    void *tx_buffer;
    unsigned tx_data_len;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&server_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(LISTEN_SOCKET_PORT);

    printf("Trying to setup listen socket on Port %d ...\n", LISTEN_SOCKET_PORT);
    if ((listen_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("Failed to create listen socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("Successfully created the listen socket. Now binding ...\n");
    int err = bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err < 0) {
        printf("Failed to bind the socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Successfully bound the listen socket! Starting to listen for incoming requests ...\n");
    while (1) {
        struct sockaddr_storage client_addr; // Large enough for both IPv4 or IPv6
        socklen_t client_socklen = sizeof(client_addr);

        int rx_data_len = recvfrom(listen_socket, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_socklen);
        if (rx_data_len < 0) {
            printf("Error during receiving on listen socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
            
        PowerMeasurement message = POWER_MEASUREMENT__INIT;
        message.timestamp = latest_measurement.timestamp;
        message.current = latest_measurement.current;
        message.energy_consumption = latest_measurement.energy_consumption;

        tx_data_len = power_measurement__get_packed_size(&message);
        tx_buffer = malloc(tx_data_len);
        power_measurement__pack(&message, tx_buffer);

        int err = sendto(listen_socket, tx_buffer, tx_data_len, 0, (struct sockaddr *)&client_addr, client_socklen);
        if (err < 0) {
            printf("Error occured during sending on listen socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        free(tx_buffer);
    }   
}

static void handle_sigint(int signal) {
    listen_for_data = 0;
    return;
}

static void shutdown_sock(int sock) {
    printf("Shutting down ...\n");
    shutdown(sock, 0);
    close(sock);
}
