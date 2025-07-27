# ESP32 Smart Inventory Management

An ESP‑IDF‑based two‑ESP32 system for automated barcode‑driven item sorting, shelf occupancy tracking, and environmental monitoring (temperature, humidity, spill detection).  

![Workflow](docs/overview.png)

## Features

- **Primary Controller** (ESP32 + I²C LCD 20×4 + Wi‑Fi AP)
  - Serves as a SoftAP (`ESPBarTest` / `test1234`)
  - Receives EAN‑8 barcodes over TCP and parses into:
    - Size (Small/Medium/Large)
    - Type (Frozen/Dry)
    - Phase (Liquid/Solid)
  - Allocates shelf slots via proximity sensors (via secondary ESP)
  - Special logic:
    - **Frozen** → “Frozen Section”
    - **Liquid** → forced to “LG_spill” slot
    - Otherwise → next available slot by size
  - Displays on LCD (4 rows):
    1. Barcode
    2. `Size/Type/Phase`
    3. `Slot: XX` or `LG_spill FULL` / `Small FULL`
    4. Live temperature & humidity
  - Three LEDs (GPIO 12/13/27) indicate:
    - Temp > threshold
    - Humidity > threshold
    - Spill detected

- **Secondary Controller** (ESP32 + proximity sensors + BMP280)
  - Reads an array of IR sensors (shelf occupancy) + one “spill” sensor
  - Reads temperature & humidity (BME280/BMP280)
  - Connects as station to primary’s AP
  - Streams CSV over TCP every second:
    ```
    0,1,0,0,1, … ,<spill>,<temp>,<humidity>
    ```

## Hardware

### Primary ESP32  
- I²C LCD 20×4:  
  - SDA → GPIO 21  
  - SCL → GPIO 22  
- Buttons:  
  - SW1 (scan toggle) → GPIO 2 (pulled‑up)  
  - SW2 (show last scan) → GPIO 5 (pulled‑up)  
- LEDs (active high):  
  - Temp Alert → GPIO 12  
  - Hum Alert  → GPIO 13  
  - Spill Alert→ GPIO 27  

### Secondary ESP32  
- Proximity sensors (SHELF_SLOTS + 1 “spill” sensor) → multiplexed GPIOs  
- BME280/BMP280 + AHT20 → I²C (select your SDA/SCL pins)  
