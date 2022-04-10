# ESP32 Power Measurement Node
This module contains the code for the ESP32 powermeter which utilizes INA3221 sensors that measure the power consumption
of edge devices connected to one of the sensor's channels.<br>
After connecting to a network via WiFi, a UDP server is set up in the background which accepts channel subscription requests by clients in order to send the power measurements to the appropriate edge devices.

#### Additional Project Configuration
The `menuconfig` offers the possibility to adjust the configuration regarding the WiFi connection, the UDP server and the power measurements.


### Configuration & Usage
1. (Optional) Configure the project
```bash
idf.py menuconfig
```
2. Build the project
```bash
idf.py build
```

3. Flash the code via USB connection
```bash
# Example for windows with ESP32 connected to COM5 port
idf.py -p COM5 flash
```

4. (Optional) Monitor the application
```bash
# Example for windows with ESP32 connected to COM5 port
idf.py -p COM5 monitor
```

### Build
ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Additionally, this project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
