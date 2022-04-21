# Power Measurement UDP Client
This module contains the code and resources for the UDP client that can be used on edge devices linked to one of the INA3221 channels in order to receive the power measurements from the `esp32-powermeter-udp-server`.
After sending the request for the given INA3221 channel, the client will infinitely listen for incoming measurements.

A UDP listen socket on port `5000` is created on another thread so external sources, like the `prometheus-power-exporter`, can fetch the power measurements from the edge device via UDP.

### Usage
The required arguments are IP and port of the target UDP server as well as the channel (e.g. `2`) to fetch power measurements from.
```bash
./udp-client <UDP_IP> <UDP_PORT> <INA3221_CHANNEL>
```

### Build
The build requires `protobuf-c` to be installed as the received data is assumed to receive as a serialized protobuf.

Build with `gcc` on a linux system.
```bash
gcc -g -I/usr/include udp_client.c  ../protobuffers/powermeasurements/powermeasurement.pb-c.c -o udp_client -L/usr/lib -lprotobuf-c -lpthread
```
