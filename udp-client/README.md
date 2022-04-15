# UDP client for edge devices
This linux-based UDP client can be used on edge devices in order to connect to the ESP32 powermeter and request the power measurements from a specific INA3221 channel. After sending the request the client will infinitely listen for incoming data.

A UDP listen socket on port `5000` is created on another thread so external sources, like the `prometheus-power-exporter`, can fetch the power measurements from the edge device via UDP.

### Usage
The required arguments are IP and port of the target UDP server as well as the channel (e.g. `2`) to fetch power measurements from.
```bash
./udp-client <UDP_IP> <UDP_PORT> <INA3221_CHANNEL>
```

### Build
The build requires `protobuf-c` to be installed.

Build with `gcc` on a linux system.
```bash
gcc -g -I/usr/include udp_client.c  ../protobuffers/powermeasurements/powermeasurement.pb-c.c -o udp_client -L/usr/lib -lprotobuf-c -lpthread
```
