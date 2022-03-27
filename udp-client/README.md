# UDP client for edge devices
This linux-based UDP client can be used on the edge devices in order to connect to the ESP32 powermeter that sends the power consumption measurements.


### Configuration & Usage
IP and port of the target UDP server are passed as arguments:
```bash
./udp-client <IP> <Port>
```

### Build
ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Additionally, this project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
