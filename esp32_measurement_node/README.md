# ESP32 & INA3221
This contains the code for the ESP32 powermeter which is used to measure the power consumption of the edge devices connected to its INA3221 sensor.

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
