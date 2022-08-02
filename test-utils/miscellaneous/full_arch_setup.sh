#!/bin/bash

# This will create the docker containers needed to deploy both the powermeasurement client and exporter
# for every supported system architecture.
#
# The powermeasurement clients will receive their data from a mocked powermeasurement server which is deployed
# as a docker container.
# Each exporter will automatically be configured to communicate with the powermeasurement client of the 
# same architecture.
#
# As soon as they are not needed anymore, the created docker containers can be removed by executing 
# 'remove_full_arch_setup.sh'

# Create the powermeasurement server ...
echo "Creating Docker Container for mocked powermeasurement server ..."
docker run -d --name mock-powermeasurement-server phyz1x/mock-powermeasurement-server:1.0.0

MOCK_SERVER_IP=$(docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' mock-powermeasurement-server)
echo "Mock-server IP: $MOCK_SERVER_IP"


# Create ARMv7 client and exporter
echo "Creating Docker Container for ARMv7 powermeasurement client ..."
docker run -d --name armv7-client -e POWERMETER_SERVER_IP="$MOCK_SERVER_IP" -e POWERMETER_SERVER_PORT=10000 -e INA3221_CHANNEL=3 --platform linux/arm/v7 phyz1x/powermeasurement-udp-client:1.0.0

ARMV7_CONTAINER_IP=$(docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' armv7-client)
echo "ARMv7 Client IP: $ARMV7_CONTAINER_IP"

echo "Creating Docker Container for ARMv7 prometheus exporter ..."
docker run -d --name armv7-exporter -e DEVICE_UDP_IP=$ARMV7_CONTAINER_IP -e DEVICE_UDP_PORT=5000 --platform linux/arm/v7 phyz1x/prometheus-power-exporter:python

echo

# Create ARM64/v8 client and exporter
echo "Creating Docker Container for ARM64/v8 powermeasurement client ..."
docker run -d --name arm64v8-client -e POWERMETER_SERVER_IP="$MOCK_SERVER_IP" -e POWERMETER_SERVER_PORT=10000 -e INA3221_CHANNEL=3 --platform linux/arm64/v8 phyz1x/powermeasurement-udp-client:1.0.0

ARM64V8_CONTAINER_IP=$(docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' arm64v8-client)
echo "ARM64/v8 Client IP: $ARM64V8_CONTAINER_IP"

echo "Creating Docker Container for ARM64v8 prometheus exporter ..."
docker run -d --name arm64v8-exporter -e DEVICE_UDP_IP=$ARM64V8_CONTAINER_IP -e DEVICE_UDP_PORT=5000 --platform linux/arm64/v8 phyz1x/prometheus-power-exporter:python

echo 

# Create AMD64 client and exporter
echo "Creating Docker Container for AMD64 powermeasurement client ..."
docker run -d --name amd64-client -e POWERMETER_SERVER_IP="$MOCK_SERVER_IP" -e POWERMETER_SERVER_PORT=10000 -e INA3221_CHANNEL=3 --platform linux/amd64 phyz1x/powermeasurement-udp-client:1.0.0

AMD64_CONTAINER_IP=$(docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' amd64-client)
echo "AMD64 Client IP: $AMD64_CONTAINER_IP"

echo "Creating Docker Container for AMD64 prometheus exporter ..."
docker run -d --name amd64-exporter -e DEVICE_UDP_IP=$AMD64_CONTAINER_IP -e DEVICE_UDP_PORT=5000 --platform linux/amd64 phyz1x/prometheus-power-exporter:python
