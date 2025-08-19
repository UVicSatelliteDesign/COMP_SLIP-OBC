🛰️ Overview

The Satellite OBC firmware implements high-level and low-level tasks for controlling the payload, managing power, and ensuring reliable communication with the ground. It is written in C for STM32 microcontrollers and runs under FreeRTOS.

The OBC integrates sensors, cameras, GPS, flash memory, and battery management to support scientific payload operations and telemetry collection.

🔧 Core Responsibilities

Task-based Operation (FreeRTOS)

obc_notifications: handles mode changes, GPS requests, camera triggers.

data_task: logs telemetry data periodically.

low_power_task: monitors charge and altitude, enabling energy conservation.

image_task: captures dual-camera snapshots every 30 seconds.

Battery Management System (BMS)

Tracks voltage, current, temperature.

Computes derived values: state-of-charge (SoC), power usage, cumulative energy, remaining life.

Stores values persistently to flash using BatteryData struct.

Sensor Integration

Polls temperature, pressure (MS561101BA03-50), gyroscope, accelerometer (KX134-1211).

Validates data with magic numbers in flash.

GPS Subsystem

Interfaced via UART (MAX-M10S).

Collects $GPRMC sentences with position, course, and time.

Provides 84-bit compressed representation for storage.

Camera Subsystem

Dual OV5640 cameras with configurable resolution, pixel format, brightness, and light mode.

Image capture pipeline:

Configure with I²C.

Capture snapshot into shared buffer.

Validate size via JPEG markers.

Store in external memory.

Flash Interface

General-purpose API for write, read, and erase across sectors 1–7.

Used by BMS, telemetry, and image modules.

Telemetry, Tracking & Command (TTC)

Implements task notifications.

Supports ACK/NACK retransmission timer.

Handles communication state transitions (Nominal, Idle, Lost).

📑 System Architecture

📂 Key Files
satellite/
├── Core/Inc/
│   ├── obc.h
│   ├── camera.h
│   ├── obc_interface.h
│   ├── flash_interface.h
│   ├── ttc_interface.h
│   └── main.h
├── Core/Src/
│   ├── obc.c
│   ├── camera.c
│   ├── obc_interface.c
│   ├── flash_interface.c
│   ├── ttc_interface.c
│   └── main.c
└── drivers/
