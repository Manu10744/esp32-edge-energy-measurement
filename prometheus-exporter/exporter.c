#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "microhttpd.h"
#include "prom.h"
#include "promhttp.h"

#define HTTP_DAEMON_PORT 8000

static void init_exporter();
static void handle_sigint(int signal);

static volatile sig_atomic_t running = 1;

struct MHD_Daemon *http_daemon;

int main(int argc, char *argv[]) { 
    init_exporter();

    printf("Starting HTTP daemon on port %d in the background.\n", HTTP_DAEMON_PORT);
    http_daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTP_DAEMON_PORT, NULL, NULL);
    if (http_daemon == NULL) {
        printf("\nFailed to start the exporter daemon!\n");
        return EXIT_FAILURE;
    }

    struct sigaction handler;
    handler.sa_handler = handle_sigint;
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0) {
        printf("\nFailed to register the SIGINT handler!\n");
        return EXIT_FAILURE;
    }

    while (running) {
        // Expose metrics ... Already done by the daemon.
    }

    printf("\nShutting down ...\n");
    return EXIT_SUCCESS;
}

static void init_exporter() {
    // Initialize the Default registry
    int err = prom_collector_registry_default_init();
    if (err) {
        printf("\nFailed to initialize the default collector registry!\n");
        exit(EXIT_FAILURE);
    }
    printf("Successfully initialized the default collector registry.\n");

    // TODO: Initalize the metrics
}

static void handle_sigint(int signal) {
    prom_collector_registry_destroy(PROM_COLLECTOR_REGISTRY_DEFAULT);
    MHD_stop_daemon(http_daemon);
    running = 0;
}