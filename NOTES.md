# Configuration Adjustments
- Dynamic Voltage & Frequency Scaling (DVFS)<br>
See also https://wiki.archlinux.org/title/CPU_frequency_scaling

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

The files `cpuinfo_*` (for example `cpuinfo_cur_freq`) rather have more to do with the specification of the CPU and which profile it's currently in, rather than anything useful with respect to how the CPU is currently operating. For actual operational telemetry I'd use the `scaling_*` kernel tunables.


- Display current CPU frequency

    Displayed in Hertz.
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
    ```

- Display max CPU frequency

    Displayed in Hertz.
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
    ```

- Display min CPU frequency (Desired Idle Freq.)

    Displayed in Hertz.
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
    ```


## Raspberry Pi
<br>

### ODROID-XU4
According to https://wiki.odroid.com/odroid-xu4/application_note/software/cpufrequtils_cpufreq_govornor, the CPU frequency can be adjusted using the tool `cpufrequtils`.


<br>

### Google Coral Dev Board

<br>

### Jetson Nano

<br>