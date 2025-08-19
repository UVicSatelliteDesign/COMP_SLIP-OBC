# 🛰️ Satellite Firmware — University of Victoria Satellite Design

## 📖 Overview
This repository contains the **On-Board Computer (OBC) firmware** for the UVic Satellite Design project. The OBC runs on an STM32 microcontroller under **FreeRTOS** and is responsible for managing payload operations, sensors, cameras, power systems, and communication with the TTC (Telemetry, Tracking, and Command).

The firmware is designed to be **modular, fault-tolerant, and power-aware**, enabling autonomous satellite operation across multiple mission modes (Idle, Nominal, Low Power).

---

## 🔧 Core Responsibilities
- **Task Scheduling (FreeRTOS)**  
  - `obc_notifications`: handles mode changes, camera requests, GPS polling.  
  - `data_task`: collects telemetry periodically and stores to flash.  
  - `low_power_task`: monitors charge/altitude → triggers Low Power Mode.  
  - `image_task`: captures snapshots from dual cameras every 30s.  

- **Battery Management System (BMS)**  
  - Monitors voltage, current, temperature.  
  - Computes SoC, power draw, cumulative energy, remaining runtime.  
  - Saves persistent `BatteryData` struct to flash with integrity checks.  

- **Sensor Subsystem**  
  - Temperature, pressure, gyroscope, and accelerometer polling.  
  - Hardware abstraction layer for upper-level OBC tasks.  

- **GPS Integration**  
  - UART interface with MAX-M10S.  
  - Collects `$GPRMC` sentences (position, velocity, time).  
  - Compact 84-bit storage format for flash memory.  

- **Camera Subsystem (OV5640)**  
  - Dual-camera system (I²C + DCMI interface).  
  - Configurable resolution, pixel format, brightness, effects.  
  - JPEG compression and buffer management.  
  - Snapshot pipeline → buffer → flash/SD.  

- **Flash Interface**  
  - Abstracted read, write, and erase functions.  
  - Used by BMS, telemetry logger, and camera.  

- **Telemetry, Tracking & Command (TTC)**  
  - Notification-driven logic for transmitting/receiving data.  
  - Implements ACK/NACK with retransmission timer.  
  - Handles transitions to “Lost” state and recovery via Ping-ACK.  

---

## 🗂️ File Structure
├── Core/Inc/
│ ├── obc.h # OBC task definitions
│ ├── camera.h # Camera subsystem
│ ├── obc_interface.h # Sensor interface
│ ├── flash_interface.h # Flash read/write/erase
│ ├── ttc_interface.h # TTC communication
│ └── main.h # Notifications & system defines
├── Core/Src/
│ ├── obc.c
│ ├── camera.c
│ ├── obc_interface.c
│ ├── flash_interface.c
│ ├── ttc_interface.c
│ └── main.c # FreeRTOS task initialization


---

## 📑 System Architecture
Below are representative diagrams from the documentation:

### OBC Task Flow
![OBC Task Flow](images/page2_img1.png)

### Battery Management Data Cycle
![Battery Management Flow](images/page9_img1.png)

### Camera Interface
![Camera Subsystem](images/page37_img1.png)

---

## ⚙️ Features
- **Modes:** Idle, Nominal, Low Power.  
- **Persistent Storage:** Flash + SD card integration.  
- **Robust Error Handling:** For GPS, camera, memory, and comm failures.  
- **Power Efficiency:** Automatic switching to Low Power mode when thresholds reached.  
- **Extendability:** Modular design for adding new payloads or peripherals.  

---

## 🚀 Next Steps
- Finalize flash sector addressing for mission hardware.  
- Integrate with full TTC link (CC1201 transceiver).  
- Expand hardware-in-loop testing coverage (GPIO + peripherals).  
