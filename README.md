# Power Measurement Framework for edge-enabled IoT devices

Measure the power consumption of serverless functions executed on edge-enabled devices and push the data to the existing Function Delivery Network (FDN) infrastructure (Prometheus & Grafana).<br>
The power consumption measurements are carried out by a ESP32 powermeter.

<br>

### Table of Contents
#### Main components
- `udp_client`: Contains the source code for the UDP client that is used.
- `udp_server`: Contains the source code for the UDP server that is used.
- `functions`: Contains the serverless function(s) to apply for tests.

#### Test utils
- `test-utils`: scripts and utilities to test functionality.

#

### Useful links
- [Espressif ESP32 Development Environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
- [VSCode ESP32 IDF Plugin Docs](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/toc.md)

#

### TO-DOs
- [ ] Setup UDP connection between ESP32 and Jetson Nano
