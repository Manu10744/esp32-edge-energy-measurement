# Power Consumption Exporter for Prometheus

This module contains the code and resources for the prometheus exporter responsible for exposing the power consumption related metrics of a specific edge device.<br>
The exporter fetches the necessary data from the monitored device using UDP communication, updates the metrics accordingly and exposes them using a HTTP daemon in the format required by prometheus.

<br>

### Dependencies

The exporter is built on top of the [Official Python Prometheus Client](https://github.com/prometheus/client_python).<br>
Additionally, `Protocolbuffers` are used for the communication between a target device and its exporter - the implementation used is https://github.com/danielgtaylor/python-betterproto. Both can be installed with `pip`:

```bash
pip install prometheus_client
pip install betterproto
```

<br>

### Prometheus Metrics

| Name | Type | Description |
| ---  |  --- |    ---      |
|  powerexporter_power_consumption_ampere_seconds_total    |  Counter    |       The total power consumption of the target device in As (ampere-seconds).      | 
|   powerexporter_current_ampere   |  Gauge    |    The measured electric current in A (ampere).         | 


### Access metrics
The exported metrics can be accessed on port `8000` by executing an `HTTP GET` request to the `/metrics` endpoint.
```bash
curl 127.0.0.1:8000

Response:
OK

curl 127.0.0.1:8000/metrics

Response:
# HELP powerexporter_power_consumption_ampere_seconds_total The total power consumption given in ampere-seconds.
# TYPE powerexporter_power_consumption_ampere_seconds_total counter
powerexporter_power_consumption_ampere_seconds_total 304

# HELP powerexporter_current_ampere The electric current given in ampere.
# TYPE powerexporter_current_ampere gauge
powerexporter_current_ampere 1.5999999046325684
```

<br>

### Usage 
```bash
# Execute either with arguments ...
./exporter <UDP_IP> <UDP_PORT>

# ... or by using ENVs
export DEVICE_UDP_IP="<UDP_IP>"
export DEVICE_UDP_PORT="<UDP_PORT>"
./exporter 
```

<br>

### Docker
|  OS     |  Architecture            |
| ---     |      ---                 |
| Debian 11  |  AMD64, ARMv7, ARM64/v8   |  

#### Build the Docker Image:

- **Note:** Docker `buildx` is required as this will build the image for multiple architectures.
```bash
make dockerimage
```

#### Run the Docker Image:
```bash
docker run --name prometheus-power-exporter -e DEVICE_UDP_IP=127.0.0.1 -e DEVICE_UDP_PORT=5000 phyz1x/prometheus-power-exporter:latest
```