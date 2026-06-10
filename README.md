# ESP32-C6 VL53L0X Time-of-Flight (ToF) Distance Measurement Project

This repository contains an energy-optimized implementation for distance measurement using an **ESP32-C6** microcontroller and the **STMicroelectronics VL53L0X** Time-of-Flight sensor. It leverages a fast-injection calibration process to minimize boot times and power consumption, making it ideal for battery-operated applications.

## Key Features

- **Fast Injection Calibration:** Skips the standard ~40ms device initialization and SPAD management by directly injecting pre-calculated calibration parameters into the registers, reducing setup time to ~1ms.
- **Hardware Standby Mode:** Completely powers down the sensor using the `XSHUT` pin, dropping standby current to less than 5 µA.
- **Robust Averaging Filter:** Collects a batch of measurements (e.g., 20 samples) with error checking to filter out range anomalies or timeouts.
- **ESP-IDF Native C++ Driver:** Clean object-oriented abstraction wrapping the official STMicroelectronics API.

## Hardware Architecture

### Pin Configuration

| ESP32-C6 Pin | VL53L0X Pin | Signal Name | Description |
| :--- | :--- | :--- | :--- |
| **GPIO 6** | Pin 9 | `SDA` | I2C Serial Data (External pull-ups recommended) |
| **GPIO 7** | Pin 10 | `SCL` | I2C Serial Clock (Up to 400 kHz Fast-Mode) |
| **GPIO 8** | Pin 5 | `XSHUT` | Hardware Standby / Reset (Active LOW) |
| **3.3V** | Pin 1 & 11 | `VCC` / `AVDD` | Power Supply (2.6 V to 3.5 V) |
| **GND** | Pins 2,3,4,6,12| `GND` | Common Ground |

---

## Code Overview

- **`main.cpp`**: Contains the application entry point (`app_main`), static calibration constants, and the execution loop that wakes the sensor, takes a measurement batch, logs the average, and enters hardware standby.
- **`VL53L0X.h`**: Header file containing the C++ class wrapper for the VL53L0X sensor. It handles `init()`, `fastInit()`, `read()`, and power control tracking.

### System Flow

1. **System Boot**: ESP32-C6 initializes the I2C master bus at 400 kHz.
2. **Sensor Wake-up**: The `XSHUT` pin is pulled HIGH.
3. **Calibration Injection**: `sensor.fastInit(...)` quickly writes saved parameters.
4. **Data Acquisition**: A loop gathers multiple individual range samples.
5. **Data Processing**: Computes a clean arithmetic mean of valid readings.
6. **Hardware Shutdown**: The `XSHUT` pin is pulled LOW, cutting off the sensor's MCU.

---

## Calibration Guide

The accuracy of the ToF sensor depends on target distance, environmental temperature, and any protective cover glass. The code utilizes predefined parameters:

```cpp
const uint32_t cal_refSpadCount = 3;
const uint8_t cal_isApertureSpads = 0;   
const uint8_t cal_VhvSettings = 31;
const uint8_t cal_PhaseCal = 1;
const int32_t cal_offsetMicroMeter = 21000; // 21 mm Offset
const FixPoint1616_t cal_xTalk = 0;
```

### How to Re-Calibrate

1. In `main.cpp`, temporarily replace `sensor.fastInit(...)` inside your measurement pipeline with the standard `sensor.init()` call.
2. Place a target at an exact known distance (e.g., 100 mm) in front of the sensor.
3. Call `sensor.performOffsetCalibration(100);` followed by `sensor.printCalibrationData();`.
4. Capture the calibrated values from the serial console monitor and update the global constants in `main.cpp`.

---

## Getting Started

### Prerequisites

* [ESP-IDF v5.1 or newer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/)
* VL53L0X API official component sources included in your build paths.

### Building & Flashing

1. Configure your target to ESP32-C6:
```bash
idf.py set-target esp32c6

```


2. Build the project:
```bash
idf.py build

```


3. Flash the binary and open the serial monitor:
```bash
idf.py -p <PORT> flash monitor

```
