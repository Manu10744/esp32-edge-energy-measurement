# UDP client for edge devices
This linux-based UDP client can be used on the edge devices in order to connect to the ESP32 powermeter that sends the power consumption measurements.


### Configuration & Usage
IP and port of the target UDP server are passed as arguments:
```bash
./udp-client <IP> <Port>
```

### Build
Build with `gcc` on a linux system.
```bash
gcc -fdiagnostics-color=always -g udp_client.c -o ./udp_client
```
