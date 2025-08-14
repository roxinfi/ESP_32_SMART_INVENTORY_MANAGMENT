# 📦 ESP32 Smart Inventory Management System

![ESP32](https://img.shields.io/badge/Platform-ESP32-blue)
![ESP-IDF](https://img.shields.io/badge/Framework-ESP--IDF-green)
![Status](https://img.shields.io/badge/Status-Active-success)

## 📖 Overview
The **ESP32 Smart Inventory Management System** is a dual-controller IoT solution for **real-time inventory tracking and environmental monitoring**.  
It integrates **barcode-based slot allocation**, **sensor-based monitoring**, and **visual alerts** to automate stock management in retail, warehousing, and cold storage applications.

**System Components:**
1. **Primary Controller** – Displays inventory information, processes barcode scans, manages slot assignments, and shows alerts.
2. **Secondary Controller** – Reads sensor data (slot occupancy, temperature, humidity, spill detection) and sends it via TCP to the Primary Controller.

---

## ✨ Features

### 🖥 Primary Controller
- 20×4 I²C LCD for live inventory display.
- TCP server for:
  - Barcode scanner input.
  - Sensor data reception from Secondary Controller.
- Button controls:
  - **GPIO 2** – Toggle scan mode.
  - **GPIO 5** – Show last scanned item.
- Alert LEDs:
  - **GPIO 12** – Temperature alert.
  - **GPIO 13** – Humidity alert.
  - **GPIO 27** – Spill alert.
- Wi-Fi **SoftAP** mode (`ESPBarTest` / `test1234`).

### 📡 Secondary Controller
- Multiplexed IR sensors for slot detection.
- Environmental sensors:
  - **AHT20** (temperature & humidity).
  - **BMP280/BME280** (temperature & pressure).
- Spill detection sensor.
- TCP client to Primary Controller.
- Sends CSV-formatted data every 1 second.

---

## 🏗 Project Structure
ESP_32_SMART_INVENTORY_MANAGMENT/
│
├── ESP_LCD_20X4/               # Primary Controller Firmware
│   ├── main/                   # LCD control, TCP server, allocation logic
│   ├── CMakeLists.txt
│   └── sdkconfig
│
├── ESP_32_SENSOR_TRANSMITTER/  # Secondary Controller Firmware
│   ├── main/                   # Sensor reading & TCP client code
│   ├── CMakeLists.txt
│   └── sdkconfig
│
└── README.md                   # Project documentation

---

## 🖥 Platform & Dependencies
- **MCU:** ESP32
- **Framework:** [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/)
- **Language:** C
- **LCD Library:** [Operating 20x4 I2C LCD with ESP32](https://github.com/MuhammadRaheelNaseem/Operating-20x4-I2C-LCD-With-ESP32)
- **Sensor Library:** [ESP AHT20 BMP280](https://github.com/peff74/ESP_AHT20_BMP280)

---

## 📥 Installation

### 1️⃣ Clone the repository
```bash
git clone https://github.com/roxinfi/ESP_32_SMART_INVENTORY_MANAGMENT.git
cd ESP_32_SMART_INVENTORY_MANAGMENT
```

### 2️⃣ Build and Flash

#### Primary Controller
```bash
cd ESP_LCD_20X4
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```

#### Secondary Controller
```bash
cd ESP_32_SENSOR_TRANSMITTER
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```
> Replace `<PORT>` with your ESP32's COM/serial port.

---

## 🔌 Hardware Pinout

### Primary Controller
| Component            | Pin(s)           |
| -------------------- | ---------------- |
| LCD (I²C)            | SDA: 21, SCL: 22 |
| LED – Temp Alert     | GPIO 12          |
| LED – Humidity Alert | GPIO 13          |
| LED – Spill Alert    | GPIO 27          |
| Button – Scan Toggle | GPIO 2           |
| Button – Show Last   | GPIO 5           |

### Secondary Controller
| Component     | Pin(s)                 |
| ------------- | ---------------------- |
| AHT20         | I²C pins as configured |
| BMP280/BME280 | I²C pins as configured |
| IR Sensors    | Multiplexed GPIO setup |
| Spill Sensor  | Configured GPIO        |

---

## 📡 Network Configuration

**Primary Controller – SoftAP Mode**
- **SSID:** `ESPBarTest`
- **Password:** `test1234`

**Secondary Controller – TCP Client**
- Connects to Primary AP.
- Sends data in CSV format:
```
slot1,slot2,...,spill,tempC,humidity
```

---

## 🛠 Troubleshooting

| Issue                 | Possible Cause           | Fix                             |
| --------------------- | ------------------------ | ------------------------------- |
| LCD not displaying    | Wrong I²C address        | Scan & update address in code   |
| No sensor data        | Network drop or wrong IP | Verify Wi-Fi and TCP settings   |
| Wrong barcode parsing | Bad checksum             | Implement EAN-8 validation      |
| LEDs always on        | Bad thresholds           | Adjust thresholds in code       |
| Build errors          | ESP-IDF mismatch         | Install correct ESP-IDF version |

---

## 📚 References & Acknowledgements
This project uses or is inspired by:
- Muhammad Raheel Naseem – *Operating 20x4 I2C LCD with ESP32*  
  https://github.com/MuhammadRaheelNaseem/Operating-20x4-I2C-LCD-With-ESP32
- peff74 – *ESP AHT20 BMP280*  
  https://github.com/peff74/ESP_AHT20_BMP280

---

## 📜 License
This project is developed as part of the **Fanshawe College – London, Ontario**  
**Embedded Systems Development 3 (ELNC-6012)** course.  
It is intended solely for **educational and academic purposes**.

- Use this project **at your own discretion and risk**.  
- **Not for commercial use** or redistribution without written permission.  
- The authors and Fanshawe College accept **no liability** for any misuse, damages, or consequences arising from the use of this code.

© 2025 Fanshawe College – All Rights Reserved.
