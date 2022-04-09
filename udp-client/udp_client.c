#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "udp_client.h"

#define IP_PROTOCOL 0
#define RECEIVE_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 16

static void handle_sigint(int signal);
static void process_data(char rx_data[]);
static void shutdown_sock(int sock);

static volatile sig_atomic_t listen_for_data = 1; 

int main(int argc, char *argv[]) {
    char server_ip[50];
    uint16_t port;
    uint8_t requested_channel;

    if (argc != 4) {
        printf("\nUsage: %s <server_ip> <server_port> <channel>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct sigaction handler;
    handler.sa_handler = handle_sigint;
    handler.sa_flags = 0;

    if (sigaction(SIGINT, &handler, NULL) != 0) {
        printf("Failed to register the SIGINT handler!\n");
        return EXIT_FAILURE;
    }

    strcpy(server_ip, argv[1]);
    port = strtoul(argv[2], NULL, 10);
    requested_channel = strtoul(argv[3], NULL, 10);

    printf("Targeting server on IP: %s | Port: %d and requesting data for channel %d!\n", server_ip, port, requested_channel);
    start_udp_communication(server_ip, port, requested_channel);
}

/**
 * Communicates with the server identified by the given IP address and port
 * via UDP protocol.
 * 
 * The client initially sends the desired channel ID to the UDP server and starts
 * to listen for incoming power measurement data.
 * 
 * @param server_ip the IP of the target server.
 * @param port the port of the target server.
 * @param requested_channel the channel to request power measurements from.
 */
void start_udp_communication(char server_ip[], uint16_t port, uint8_t requested_channel) {
    int client_socket = 0;
    int rx_data_len;
    char rx_buffer[RECEIVE_BUFFER_SIZE] = {0};
    char tx_buffer[SEND_BUFFER_SIZE] = {0};
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);   

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nThe address %s is either invalid or not supported!\n", server_ip);
        exit(EXIT_FAILURE);
    }
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)) < 0) {
        printf("\nCould not create socket!\n");
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed!\n");
        shutdown_sock(client_socket);
        exit(EXIT_FAILURE);
    }
    
    int tx_data_len = sprintf(tx_buffer, "%u", requested_channel);
    send(client_socket, tx_buffer, tx_data_len + 1, 0);

    while (listen_for_data) {
        rx_data_len = read(client_socket, rx_buffer, RECEIVE_BUFFER_SIZE);
        if (rx_data_len < 0) {
            printf("Error during receiving!\n");
            break;
        }

        printf("Received %d bytes from %s: %s\n", rx_data_len, server_ip, rx_buffer);
        process_data(rx_buffer);
    }

    shutdown_sock(client_socket);
}

/**
 * Processes the datagrams representing the power measurements received by 
 * the UDP server.
 * 
 * @param rx_data the received data that should be processed.
 */
static void process_data(char rx_data[]) {
    char *eptr;
    unsigned long long energy_consumption = strtoull(rx_data, &eptr, 10);

    printf("Energy Consumption: %llu mAs\n", energy_consumption / 1000000);
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
