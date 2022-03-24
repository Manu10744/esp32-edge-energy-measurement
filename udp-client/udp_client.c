#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define UDP_SERVER_IP "127.0.0.1"
#define UDP_SERVER_PORT 3333

#define IP_PROTOCOL 0

#define RECEIVE_BUFFER_SIZE 1024

void connect_to_server();

/**
 * Connects to the UDP server defined by UDP_SERVER_IP and UDP_SERVER_PORT.
 */
void connect_to_server() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char hello_msg[] = "Hello from the client!";
    char reicvbuf[RECEIVE_BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)) < 0) {
        printf("\nCould not create socket!\n");
        return -1;
    }

    serv_addr.sin_addr = AF_INET;
    serv_addr.sin_port = htons(UDP_SERVER_PORT);

    if (inet_pton(INET_AF, UDP_SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nThe address %s is either invalid or not supported!\n", UDP_SERVER_IP);
        return -1;
    }

    printf("\nTrying to connect to the server ...\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed!\n");
        return -1;
    }

    send(sock, hello_msg, strlen(hello_msg), 0);
    printf("\nThe hello message was sent.\n");

    valread = read(sock , reicvbuf, RECEIVE_BUFFER_SIZE);
    printf("%s\n", reicvbuf);
    return 0;
}

void main() {
    connect_to_server();
}
