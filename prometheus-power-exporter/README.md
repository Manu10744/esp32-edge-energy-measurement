# Power Consumption Exporter for Prometheus

This module contains the code and resources for the prometheus exporter that exposes the power consumption data of a specific edge device.

It is built on top of the [Prometheus C Client by DigitalOcean](https://github.com/digitalocean/prometheus-client-c).

### Build

The build requires the following dependencies to be installed:
- libprom
- libpromhttp
- libmicrohttpd

```bash
# Build
gcc -Wall exporter.c metric_power_consumption.c -o exporter -lprom -lpromhttp -lmicrohttpd

# Execute
./exporter
```