# 📦 ESP32 Smart Inventory Management System

![ESP32](https://img.shields.io/badge/Platform-ESP32-blue)
![ESP-IDF](https://img.shields.io/badge/Framework-ESP--IDF-green)
![License](https://img.shields.io/badge/License-MIT-orange)
![Status](https://img.shields.io/badge/Status-Active-success)

## 📖 Overview
The **ESP32 Smart Inventory Management System** is a dual-controller IoT solution for **real-time inventory tracking and environmental monitoring**.  
It combines **barcode-based slot allocation**, **sensor-based monitoring**, and **visual alerts** to automate stock management in retail, warehousing, and cold storage applications.

**System Composition:**
1. **Primary Controller** – Displays inventory and alerts, processes barcode scans, and manages slot assignments.
2. **Secondary Controller** – Reads sensor data (slot occupancy, temperature, humidity, spill detection) and sends it over TCP.

---

## ✨ Features

### 🖥 Primary Controller
- 20×4 I²C LCD for live inventory data
- TCP server for:
  - Barcode scanner input
  - Sensor data reception from Secondary
- Button controls:
  - **GPIO 2** – Toggle scan mode
  - **GPIO 5** – Show last scanned item
- Alert LEDs:
  - **GPIO 12** – Temperature alert
  - **GPIO 13** – Humidity alert
  - **GPIO 27** – Spill alert
- Wi-Fi **SoftAP** mode (`ESPBarTest` / `test1234`)

### 📡 Secondary Controller
- Multiplexed IR sensors for slot detection
- Environmental sensors:
  - **AHT20** (temperature & humidity)
  - **BMP280/BME280** (temperature & pressure)
- Spill detection sensor
- TCP client to Primary
- Sends CSV-formatted data every 1 second

---

## 🏗 Project Structure
