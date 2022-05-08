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

#include <confuse.h>
#include <protobuf-c/protobuf-c.h>

#include "udp_client.h"
#include "powermeasurement.pb-c.h"

#define CONFIG_FILE_NODE_SECTION_ID "node"
#define CONFIG_FILE_POWERMETER_SERVER_ID "powermeter_server"
#define CONFIG_FILE_POWERMETER_PORT_ID "powermeter_port"
#define CONFIG_FILE_CHANNEL_ID "channel"

#define ENV_VAR_CONFIG_FILE "POWERMONITORING_CONFIG_PATH"
#define ENV_VAR_NODENAME "NODENAME"

#define ENV_VAR_IP "POWERMETER_SERVER_IP" 
#define ENV_VAR_PORT "POWERMETER_SERVER_PORT"
#define ENV_VAR_CHANNEL "INA3221_CHANNEL"

#define IP_PROTOCOL 0
#define RECEIVE_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 16

#define LISTEN_SOCKET_PORT 5000

static void handle_sigint(int signal);
static void shutdown_sock(int sock);
static void listen_for_requests();
static cfg_t* load_configuration(char config_path[], char node_id[]);

static volatile sig_atomic_t listen_for_data = 1; 

PowerMeasurement latest_measurement;

int main(int argc, char *argv[]) {
    char server_ip[50];
    uint16_t port;
    uint8_t requested_channel;
    pthread_t listen_thread_id;

    if (getenv(ENV_VAR_CONFIG_FILE)) {
        if (!getenv(ENV_VAR_NODENAME)) {
            printf("Failed to load the configuration file - node name is not set! The node name must be set by ENV '%s'.\n", ENV_VAR_NODENAME);
            return EXIT_FAILURE;
        }

        cfg_t *full_cfg = load_configuration(getenv(ENV_VAR_CONFIG_FILE), getenv(ENV_VAR_NODENAME));

        strcpy(server_ip, cfg_getstr(full_cfg, CONFIG_FILE_POWERMETER_SERVER_ID));
        port = cfg_getint(full_cfg, CONFIG_FILE_POWERMETER_PORT_ID);
        requested_channel = cfg_getint(full_cfg, CONFIG_FILE_CHANNEL_ID);
    } else {
        if (argc == 1) {
            // Determine IP, port and channel via ENVs...
            if (!getenv(ENV_VAR_IP) || !getenv(ENV_VAR_PORT) || !getenv(ENV_VAR_CHANNEL)) {
                printf("\nWrong usage - ENVs '%s', '%s' and '%s' must be set when no arguments are given!\n", ENV_VAR_IP, ENV_VAR_PORT, ENV_VAR_CHANNEL);
                return EXIT_FAILURE;
            }
            strcpy(server_ip, getenv(ENV_VAR_IP));
            port = strtoul(getenv(ENV_VAR_PORT), NULL, 10);
            requested_channel = strtoul(getenv(ENV_VAR_CHANNEL), NULL, 10);
        } else if (argc == 4) {
            // Determine IP, port and channel via args ...
            strcpy(server_ip, argv[1]);
            port = strtoul(argv[2], NULL, 10);
            requested_channel = strtoul(argv[3], NULL, 10);
        } else {
            printf("\nWrong usage - Execute %s <server_ip> <server_port> <channel>\n", argv[0]);
            printf("Alternatively, execute %s with ENVs '%s', '%s' and '%s' being set.\n", argv[0], ENV_VAR_IP, ENV_VAR_PORT, ENV_VAR_CHANNEL);
            return EXIT_FAILURE;
        }
    }

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
 * Starts the UDP communication with the server sending the power measurements.
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
    uint8_t rx_buffer[RECEIVE_BUFFER_SIZE];
    size_t rx_data_len;
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
    
    size_t tx_data_len = sprintf(tx_buffer, "%u", requested_channel);
    send(client_socket, tx_buffer, tx_data_len + 1, 0);

    while (listen_for_data) {
        rx_data_len = read(client_socket, rx_buffer, RECEIVE_BUFFER_SIZE);
        if (rx_data_len < 0) {
            printf("Error during receiving: %s\n", strerror(errno));
            break;
        }
        printf("Received %ld bytes from %s", rx_data_len, server_ip);

        PowerMeasurement *measurement = power_measurement__unpack(NULL, rx_data_len, rx_buffer);
        if (measurement == NULL) {
            printf("Failed to deserialize the received data!\n");
            exit(EXIT_FAILURE);
        }
        latest_measurement = *measurement;
        power_measurement__free_unpacked(measurement, NULL);

        printf("\nTimestamp: %llu \nEnergy Consumption: %lf mAs \nCurrent: %f mA\n\n", 
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
    struct sockaddr_in server_addr;
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

/**
 * Parses the given configuration file and returns the power measurement configuration
 * for the given node.
 * 
 * @param config_path the path to the configuration file.
 * @param node_id the ID of a node defined in the configuration file.
 *
 * @return the configuration defined for the given node ID.
 */
static cfg_t* load_configuration(char config_path[], char node_id[]) {
    printf("Trying to load configuration file: %s\n", config_path);
    if (!access(config_path, F_OK) == 0) {
        printf("Failed to load the configuration file: File does not exist!\n");
        exit(EXIT_FAILURE);
    }

    cfg_opt_t node_cfg_attrs[] = {
	    CFG_STR_LIST(CONFIG_FILE_POWERMETER_SERVER_ID, NULL, CFGF_NONE),
	    CFG_INT(CONFIG_FILE_POWERMETER_PORT_ID, 1, CFGF_NONE),
        CFG_INT(CONFIG_FILE_CHANNEL_ID, 1, CFGF_NONE),
	    CFG_END()
	};

	cfg_opt_t cfg_global_attrs[] = {
	    CFG_SEC(CONFIG_FILE_NODE_SECTION_ID, node_cfg_attrs, CFGF_TITLE | CFGF_MULTI),
	    CFG_END()
	};

	cfg_t *full_cfg;	
	full_cfg = cfg_init(cfg_global_attrs, CFGF_NONE);
	if (cfg_parse(full_cfg, config_path) == CFG_PARSE_ERROR) {
        printf("Failed to parse the configuration file!\n");
        exit(EXIT_FAILURE);
    }

    cfg_t *node_cfg;
    int node_cfg_idx;
	for (node_cfg_idx = 0; node_cfg_idx < cfg_size(full_cfg, CONFIG_FILE_NODE_SECTION_ID); node_cfg_idx++) {
	    node_cfg = cfg_getnsec(full_cfg, CONFIG_FILE_NODE_SECTION_ID, node_cfg_idx);

        if (strcmp(cfg_title(node_cfg), node_id) == 0) {
            printf("Successfully loaded configuration for node '%s'!\n\n", node_id);
            return node_cfg;
        }
	}

    printf("Failed to load configuration for node '%s': No configuration was defined for this node!\n", node_id);
    exit(EXIT_FAILURE);
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
