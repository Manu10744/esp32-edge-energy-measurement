# Power Measurement Framework for edge-enabled IoT devices

Measure the power consumption of serverless functions executed on edge-enabled devices and push the data to the existing Function Delivery Network (FDN) infrastructure (Prometheus & Grafana).<br>
The power consumption measurements are carried out by a ESP32 powermeter.

### Hardware Setup

<img src="./img/hardware_setup.jpg" height="450px" width="350px">

- `INA3221`: the power measurement sensor with three channels
- `ESP32 Power Measurement Node`: the microcontroller where the main software runs
- `Jetson Nano` / `ESP32`: sample edge devices, connected to INA3221.
<br>

### Table of Contents
#### Main components
- `esp32-measurement-node`: Contains the source code for the ESP32 powermeter that measures the power consumption and sends them to the edge devices via UDP communication.
- `udp_client`: Contains the source code for the UDP client that can be used at the edge devices in order to fetch power measurements from the ESP32 powermeter.
- `prometheus-power-exporter`: Contains the resources for the prometheus exporter exposing the power consumption metrics.
- `protobuffers`: Contains the protocolbuffer messages that are used for data transmission between the individual components over the network.

#### Test utils
- `test-utils`: scripts and utilities to test functionality, especially the resources needed for a local kubernetes cluster needed for testing.
- `functions`: Contains the serverless function(s) to apply for tests.

#

### Useful links
- [Espressif ESP32 Development Environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
- [VSCode ESP32 IDF Plugin Docs](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/toc.md)
- [INA3221 Strommessung](https://www.raspberry-pi-geek.de/ausgaben/rpg/2019/02/strom-und-spannungssensor-ina3221/)
- [Prometheus metric types](https://prometheus.io/docs/concepts/metric_types/#summary)
- [Protobuffers](https://developers.google.com/protocol-buffers/docs/proto3)
#

### TO-DOs
- [x] Setup UDP connection between ESP32 and Jetson Nano
- [x] Send energy measurements to the Jetson Nano
- [x] Install Grafana & Prometheus on local k8s
- [x] Write custom prometheus exporter for the power measurements so prometheus is able to poll the data - use sample data until later
- [x] Test custom prometheus exporter in test k8s
- [x] Adjust code in powermeasurement component to measure power consumption of multiple devices (multiple channels)
- [ ] Adjust code to make it possible to use more than 1x INA3221 (-> project configuration).
- [x] Make UDP client send a certain channel ID to demand the power measurements for it instead of hardcoding channel 3 
- [x] Adjust code in UDP server to send the correct power measurements to multiple connected clients / devices
- [x] Expand the hardware setup to 1x Jetson Nano + 1x ESP32 and make sure it works
- [x] Make prometheus exporter expose the real power measurement data instead of fake data -> start with 2 simple metrics:
  - Metric 1: total energy consumption (= counter)
  - Metric 2: electrical current (= gauge)
- [ ] Transform power consumption metric to base unit ampere-seconds
- [ ] Deploy the exporter for the ESP32 and expand the grafana dashboard with a panel for the ESP32 exporter 
  - Goal: 2 running exporters exposing the real data, 2 panels in grafana visualizing it
- [x] Send the measured current to the client as well, not just the consumed energy. 
  - This will require sending structured data => Implement protocolbuffers
- [ ] [__UDP CLIENT__] Use the directory service providing info about the measurement node's IP
