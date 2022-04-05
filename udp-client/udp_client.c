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

void handle_recv(char data[]);

static volatile sig_atomic_t listen_for_data = 1;

int sock = 0;

char server_ip[50];
uint16_t port;
char channel[2]; 

/**
 * Communicates with the server identified by the given IP address and port
 * via UDP protocol.
 * 
 * The client initially sends the desired channel ID to the UDP server and starts
 * to listen for incoming power measurement data.
 * 
 * @param server_ip the IP of the target server.
 * @param port the port of the target server.
 */
void start_udp_communication(char server_ip[], uint16_t port) {
    int rx_data_len;
    struct sockaddr_in serv_addr;
    char rx_buffer[RECEIVE_BUFFER_SIZE] = {0};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);   

    if ((sock = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)) < 0) {
        printf("\nCould not create socket!\n");
        return;
    }

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nThe address %s is either invalid or not supported!\n", server_ip);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed!\n");
        return;
    }

    send(sock, channel, strlen(channel), 0);

    while (listen_for_data) {
        rx_data_len = read(sock, rx_buffer, RECEIVE_BUFFER_SIZE);
        if (rx_data_len < 0) {
            printf("Error during receiving!");
            break;
        }

        printf("Received %d bytes from %s: %s\n", rx_data_len, server_ip, rx_buffer);
        process_data(rx_buffer);
    }

    printf("Shutting down socket...\n");
    shutdown(sock, 0);
    close(sock);
}

/**
 * Processes the datagrams representing the power measurements received by 
 * the UDP server.
 * 
 * @param rx_data the received data that should be processed.
 */
void process_data(char rx_data[]) {
    char *eptr;
    unsigned long long energy_consumption = strtoull(rx_data, &eptr, 10);

    printf("Energy Consumption: %llu mAs\n", energy_consumption / 1000000);
}

void handle_sigint(int signal) {
    listen_for_data = 0;
    return;
}

int main(int argc, char *argv[]) {
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
    strcpy(channel, argv[3]);

    printf("Targeting server on IP: %s | Port: %d and requesting data for channel %s!\n", server_ip, port, channel);
    start_udp_communication(server_ip, port);
}
