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

The scaling governor can be changed by echoing the name of a different governor into the file.<br>
**Note**: only possible with root access.

```
echo <new_governor> > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
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

Additional documentation can be found in https://www.kernel.org/doc/Documentation/cpu-freq/user-guide.txt

<br>
<hr>
<br>

## Raspberry Pi

- Minimum CPU Frequency: 700 MHz
- Maximum CPU Frequency: 1200 MHz

Available scaling steps (`scaling_available_frequencies`):
- 600 MHz
- 700 MHz
- ...
- 1200 MHz

<br>

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


<br>

### Google Coral Dev Board
- Minimum CPU Frequency: 500 MHz
- Maximum CPU Frequency: 1500 MHz

Available scaling steps (`scaling_available_frequencies`):
- 500 MHz
- 1000 MHz
- 1500 MHz

<br>

### Jetson Nano

<br>