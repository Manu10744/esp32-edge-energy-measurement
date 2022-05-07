# Power Measurement UDP Client
This module contains the code and resources for the UDP client that can be used on edge devices linked to one of the INA3221 channels in order to receive the power measurements from the `esp32-powermeter-udp-server`.
After sending the request for the given INA3221 channel, the client will infinitely listen for incoming measurements.

A UDP listen socket on port `5000` is created on another thread so external sources, like the `prometheus-power-exporter`, can fetch the power measurements from the edge device via UDP.

### Build
The build requires the following dependencies to be installed:
- `protobuf-c` (requires `protobuf`)
- `libconfuse`

```bash
make build
```

### Usage

#### Using a configuration file
The UDP client can optionally be used with a configuration file, which defines the power measurement configuration for each node. This makes monitoring a whole cluster where each node uses a client fetching data for a different INA3221 channel easier.

It is expected to conform to the following format:

```text
node node01 {
	powermeter_server = "127.0.0.1"
	powermeter_port = 10000
	channel = 1
}

node node02 {
	powermeter_server = "127.0.0.1"
	powermeter_port = 10000
	channel = 2
}
```

The path of the configuration file can be specified by the environment variable `POWERMONITORING_CONFIG_PATH`.<br>
When using a configuration file, the environment variable `NODENAME` has to be set, which is used for loading the node-specific configuration from the configuration file. 

```bash
# Example
export POWERMONITORING_CONFIG_PATH="./config/monitoring-config.conf"
export NODENAME="node02"
./udp_client
```
<br>

#### Using arguments / Environment Variables
```bash
# Execute either with arguments ...
./udp_client <UDP_IP> <UDP_PORT> <INA3221_CHANNEL>

# ... or by using ENVs
export POWERMETER_SERVER_IP="<SERVER_IP>"
export POWERMETER_SERVER_PORT="<SERVER_PORT>"
export INA3221_CHANNEL="<INA3221_CHANNEL>"
./udp_client
```


### Docker
|  OS     |  Architecture            |
| ---     |      ---                 |
| Ubuntu 20.04  |  AMD64, ARMv7, ARM64/v8   |  

#### Build the Docker Image:

- **Note:** Docker `buildx` is required as the image will be built for multiple architectures.
```bash
make dockerimage
```

#### Run the Docker Image:
```bash
docker run --name powermeasurement-udp-client -e POWERMETER_SERVER_IP="127.0.0.1" -e POWERMETER_SERVER_PORT="10000" -e INA3221_CHANNEL="3" phyz1x/powermeasurement-udp-client:1.0.0
```