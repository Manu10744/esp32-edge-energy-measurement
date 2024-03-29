# Power Measurement Framework for edge-enabled IoT devices

The goal of this thesis is to setup a power monitoring infrastructure for IoT-enabled edge devices, which are operated in a serverless cluster, in order to: 

- Measure and monitor the power consumption of the individual devices.
- Be able to make statements about the power consumption of serverless functions executed on the respective device.
- Be able to make statements about how to influence the power consumption by changing configuration settings etc.

The cluster was established using Kubernetes (k8s) and the chosen FaaS platform is OpenFaaS.
The resulting cluster is integrated into the Function Delivery Network (FDN) which is a network of multiple, heterogeneous clusters.

As the FDN already uses an existing monitoring infrastructure based on Prometheus & Grafana, a custom Prometheus exporter was implemented in order to extend the monitoring with metrics related to the energy consumption of the employed edge devices.

The energy consumption measurements are carried out by a ESP32 powermeter, which measures each device individually using INA3221 sensors. 

### Energy Measurement & Monitoring Infrastructure
<img src="./thesis/figures/edge-cluster.svg" height="100%" width="100%">

<br>

### Established Monitoring
The monitoring is facilitated by a custom Grafana Dashboard, which shows the total energy consumption of all Kubernetes edge nodes (bar chart at the top) as well as the amount of energy consumed within the recent `15s` (graph at the bottom left) and the measured electrical current (graph at the bottom right).
<img src="./thesis/figures/grafana-dashboard-dark.png" height="100%" width="100%">

<br>

### Table of Contents
#### Main components
- `esp32-powermeter-udp-server`: Contains the source code for the ESP32 powermeter that measures the power consumption and sends them to the edge devices via UDP communication.
- `powermeasurement-udp-client`: Contains the source code for the UDP client that can be used at the edge devices in order to fetch power measurements from the ESP32 powermeter.
- `prometheus-power-exporter`: Contains the resources for the prometheus exporter exposing the power consumption metrics.
- `protobuffers`: Contains the protocolbuffer messages that are used for data transmission between the individual components over the network.

#### Utils
- `test-utils`: scripts and utilities to test functionality, especially the resources needed for a local kubernetes cluster needed for testing.
- `openfaas-functions`: Contains the serverless function(s) for OpenFaaS to apply for tests.
- `ansible-scripts`: Contains ansible scripts that can be used for creating an OpenFaaS edge cluster and installing the necessary tools on each node automatically.

#

### Useful links
- [Espressif ESP32 Development Environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
- [VSCode ESP32 IDF Plugin Docs](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/toc.md)
- [INA3221 Strommessung](https://www.raspberry-pi-geek.de/ausgaben/rpg/2019/02/strom-und-spannungssensor-ina3221/)
- [Prometheus metric types](https://prometheus.io/docs/concepts/metric_types/#summary)
- [Protocol Buffers - Google Docs](https://developers.google.com/protocol-buffers/docs/proto3)
- CPUfreq - Adjust CPU Clock Speed of linux-based systems
  - [CPUfreq - Official Docs](https://www.kernel.org/doc/Documentation/cpu-freq/user-guide.txt)
  - [CPUfreq - RedHat Docs](https://access.redhat.com/documentation/de-de/red_hat_enterprise_linux/6/html/power_management_guide/tuning_cpufreq_policy_and_speed)
  - [CPUfreq - Scaling governors](https://wiki.somlabs.com/index.php/How_to_scale_CPU_frequency_with_DVFS_framework)
- [Monitor System CPU Utilization with Prometheus Node Exporter](https://www.robustperception.io/understanding-machine-cpu-usage/)


#

### TO-DOs
- [x] Setup UDP connection between ESP32 and Jetson Nano
- [x] Send energy measurements to the Jetson Nano
- [x] Install Grafana & Prometheus on local k8s
- [x] Write custom prometheus exporter for the power measurements so prometheus is able to poll the data - use sample data until later
- [x] Test custom prometheus exporter in test k8s
- [x] Adjust code in powermeasurement component to measure power consumption of multiple devices (multiple channels)
- [x] Make `powermeasurement-udp-client` send a certain channel ID to demand the power measurements for it instead of hardcoding channel 3 
- [x] Adjust code in ESP32 powermeasurement server to send the correct power measurements to multiple connected clients / devices
- [x] Expand the hardware setup to 1x Jetson Nano + 1x ESP32 and make sure it works
- [x] Make `prometheus-power-exporter` expose the real power measurement data instead of fake data -> start with 2 simple metrics:
  - Metric 1: total energy consumption (= counter)
  - Metric 2: electrical current (= gauge)
- [x] Transform power consumption / eletrical current metric to base unit ampere-seconds / ampere
- [x] Deploy the exporter for the ESP32 and expand the grafana dashboard with a panel for the ESP32 exporter 
  - Goal: 2 running exporters exposing the real data, 2 panels in grafana visualizing it
- [x] Send the measured current to the client as well, not just the consumed energy. 
  - This will require sending structured data => Implement protocolbuffers
- [x] Build and upload Docker Images of `powermeasurement-udp-client` and `prometheus-power-exporter` for different architectures (ARM64v8, AMD64)
- [x] Add a dummy powermeasurement server to the LRZ Cloud Cluster as the "real" ESP32 server is not accessible in that network
- [x] Deploy `powermeasurement-udp-client` and `prometheus-power-exporter` to each worker node in the LRZ Cloud cluster
  - Extend `example.jsonnet` in `kube-prometheus` accordingly and recompile manifests
  - Ensure that everything works on Grafana's end
- [x] Add possibility to use `powermeasurement-udp-client` with a config file (needed for kubernetes `ConfigMap`)
- [x] (2/2) Add ARMv7 Docker Image for `powermeasurement-udp-client` and `prometheus-power-exporter`
- [x] (**EXTENDED HARDWARE BOARD**): Adjust the code in `esp32-power-measurement-node` in order to use both INA3221 sensors
- [x] (**EXTENDED HARDWARE BOARD**): Setup a full edge cluster with k3s and deploy the full monitoring stack to the cluster
- [x] (**EXTENDED HARDWARE BOARD**): Visualize power consumption of each device on the integrated display
  -  Idea: Display one device at a time, loop through with 1sec delay
- [x] Integrate the power consumption monitoring into the new FDN edge cluster
- [x] Deploy serverless function *analyze_sent* to OpenFaaS in FDN cluster
- [ ] Design an energy model to inference the power consumption of a device to the function invocations
