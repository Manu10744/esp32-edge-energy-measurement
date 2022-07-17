# Configuration Adjustments
- Dynamic Voltage & Frequency Scaling (DVFS)

<br>
<br>

## Raspberry Pi

- Display current CPU frequency

    Displayed in Hertz.
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
    ```

- Display max CPU frequency

    Displayed in Hertz.
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq
    ```

- Display min CPU frequency (Desired Idle Freq.)

    Displayed in Hertz.
    ```
    cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq
    ```

<br>

### ODROID-XU4

<br>

### Google Coral Dev Board

<br>

### Jetson Nano

<br>