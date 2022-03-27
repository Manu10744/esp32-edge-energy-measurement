#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "udp_client.h"

#define IP_PROTOCOL 0
#define RECEIVE_BUFFER_SIZE 1024

char server_ip[50];
uint16_t port;

/**
 * Connects to a UDP server by the given IP address and port.
 * 
 * @param server_ip the IP of the target server.
 * @param port the port of the target server.
 */
void setup_udp_connection(char server_ip[], uint16_t port) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char hello_msg[] = "Hello from the Jetson Nano!";
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

    send(sock, hello_msg, strlen(hello_msg), 0);
    while (1) {
        valread = read(sock , rx_buffer, RECEIVE_BUFFER_SIZE);
        if (valread < 0) {
            printf("Error during receiving!");
            break;
        }

        printf("Received %d bytes from %s: %s\n", valread, server_ip, rx_buffer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("\nUsage: %s <server_ip> <server_port> \n", argv[0]);
        return EXIT_FAILURE;
    } 

    strcpy(server_ip, argv[1]);
    port = strtoul(argv[2], NULL, 10);

    printf("Targeting server on IP %s and port %d!\n", server_ip, port);
    setup_udp_connection(server_ip, port);
}
