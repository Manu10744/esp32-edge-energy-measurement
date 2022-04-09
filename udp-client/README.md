# UDP client for edge devices
This linux-based UDP client can be used on the edge devices in order to connect to the ESP32 powermeter and request the power measurements from a specific INA3221 channel.<br>
Once the server received the request it will continuously send the power measurements for the edge device connected to the specified channel.

### Usage
The required arguments are IP and port of the target UDP server as well as the channel (e.g. `2`) to fetch power measurements from.
```bash
./udp-client <IP> <Port> <INA3221_Channel>
```

### Build
Build with `gcc` on a linux system.
```bash
gcc -fdiagnostics-color=always -g udp_client.c -o ./udp_client
```
