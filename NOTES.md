# Table of Contents

1. [Influencing the Power Consumption via Configuration Adjustments](#configuration-adjustments)
2. [Inference of Total Energy Consumption to Serverless Functions](#inference-of-total-energy-consumption-to-serverless-functions)

## Influencing the Power Consumption via Configuration Adjustments
- Dynamic Voltage & Frequency Scaling (DVFS)<br>
See also https://wiki.archlinux.org/title/CPU_frequency_scaling

### Scaling governors
Cited from `StackOverflow` (https://unix.stackexchange.com/questions/87522/why-do-cpuinfo-cur-freq-and-proc-cpuinfo-report-different-numbers):
<br>
There are multiple "modes" a CPU can be operated in, which is called the scaling governor. The available governors can be listed by the following command:

```
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
```
**Note:** (`cpu0` stands for the 1st CPU, so there is also `cpu1`, `cpu2` and so on.)

The current scaling governor can be listed by the following command:

```
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
```

### Change the current active scaling governor
The scaling governor can be changed by echoing the name of a different governor into the file.<br>
**Note**: only possible with root access.

```
echo <new_governor> > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

The governor `userspace` offers to set specific values for the CPU frequency. It can be selected like so:
```
echo userspace > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

After it has been selected, the user can set a specific speed for the CPU, but only within the limits of
scaling_min_freq and scaling_max_freq:

```
echo 900000 > /sys/devices/system/cpu/cpu*/cpufreq/scaling_setspeed
```

The files `cpuinfo_*` (for example `cpuinfo_cur_freq`) rather have more to do with the specification of the CPU and which profile it's currently in, rather than anything useful with respect to how the CPU is currently operating. For actual operational telemetry I'd use the `scaling_*` kernel tunables.


- Display current CPU frequency

    Displayed in Kilo-Hertz (kHz).
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
    ```

- Display max CPU frequency

    Displayed in Kilo-Hertz (kHz).
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
    ```

- Display min CPU frequency (Desired Idle Freq.)

    Displayed in Kilo-Hertz (kHz).
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
    ```

Additional documentation can be found in https://www.kernel.org/doc/Documentation/cpu-freq/user-guide.txt

### Raspberry Pi

- Minimum CPU Frequency: 700 MHz
- Maximum CPU Frequency: 1200 MHz

Available scaling steps (`scaling_available_frequencies`):
- 600 MHz
- 700 MHz
- ...
- 1200 MHz

### ODROID-XU4
- Minimum CPU Frequency: 600 MHz
- Maximum CPU Frequency: 1500 MHz on cores **1 - 4**, 20000 MHz on cores **5-8**

Available scaling steps (`scaling_available_frequencies`):
- 200 MHz
- 300 MHz
- ...
- 1400 MHz (for the cores **1 - 4**)
- ...
- 2000 MHz (for the cores **5 - 8**)

According to https://wiki.odroid.com/odroid-xu4/application_note/software/cpufrequtils_cpufreq_govornor, the CPU frequency can be adjusted using the tool `cpufrequtils`.

### Google Coral Dev Board
- Minimum CPU Frequency: 500 MHz
- Maximum CPU Frequency: 1500 MHz

Available scaling steps (`scaling_available_frequencies`):
- 500 MHz
- 1000 MHz
- 1500 MHz

### Jetson Nano


<hr>
<br>

## Inference of Energy Consumption to Serverless Functions
Useful Prometheus PromQL Queries:

- CPU Utilization of Node (1 minute)<br>
  Result is given in **Percentage**.
  ```
  100 - (avg by (instance) (rate(node_cpu_seconds_total{job="node-exporter",mode="idle"}[1m])) * 100)
  ```
- CPU Utilization of Serverless Function Container (1 minute)<br>
  Result is given in **Number of Cores**.
  ```
  (rate(container_cpu_usage_seconds_total{container="gzipcompression", image!="", container_name!="POD"}[1m]) > 0)
  ```
- Energy Consumption of Node (1 minute)<br>
  Result is given in **Ampere-Seconds (As)**
  ```
  idelta(powerexporter_power_consumption_ampere_seconds_total[2m:1m])
  ```
- Amount of CPU Cores per machine
  ```
  machine_cpu_cores
  
  # For node 'odroidxu4-2':
  machine_cpu_cores{node="odroidxu4-2"}
  ```
  
#### Issues:
`node-exporter` exposes resource usage metrics related to the system, `cAdvisor` exposes resource usage metrics related to containers. When using metrics from both sources in a formula in order to distribute the energy consumption, the following issues arise:
  - **Scraping Interval:** The interval that is used by Prometheus in order to collect the data from the individual sources are not necessarily the same for `cAdvisor` and `node-exporter` metrics. Though, this can easily be changed by configuring the same scraping interval for both targets in the Prometheus configuration.
  - **Implementation:** `cAdvisor` and `node-exporter` are not the same in terms of functionality. `cAdvisor` has its own data gathering loop and in addition to that, they can sometimes be delayed by fractions of a second. This is why the total cpu usage of a system at time `t`, as obtained from `node-exporter`, does not necessarily make sense when compared with the container cpu usage at time `t`, as obtained from `cAdvisor`.
  - **Scraping Time:**  Prometheus does not scrape both sources at the exact same time, which adds to the problem of inaccuracy. 
