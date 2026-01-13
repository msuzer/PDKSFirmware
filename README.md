# ESP32-S3 Access Control System

This project is a custom embedded firmware for an ESP32-S3–based access control device.

The system is designed for:
- RFID card reading
- Image capture
- Local storage (SD card)
- Cloud upload (Ethernet)
- Door/relay control
- User feedback via OLED, LED, and buzzer
- Persistent user preferences
- Real-time clock (RTC)

The firmware is built using **ESP-IDF** and **FreeRTOS**, managed via **PlatformIO**.

---

## Hardware Overview

- MCU: ESP32-S3 (QFN56, embedded 8MB PSRAM)
- Interfaces:
  - SPI (RFID, Ethernet)
  - I2C (OLED, RTC)
  - Parallel Camera Interface
- Peripherals:
  - MFRC522 RFID reader
  - W5500 Ethernet
  - DS3231 RTC (battery-backed)
  - OLED display (I2C)
  - Relay for door/solenoid control
  - User LED and buzzer
  - SD card

---

## Software Architecture

The firmware is structured around **services** and **drivers**:

### Core Services
- Logger (UART)
- LED service
- Buzzer service
- Relay (door control)
- I2C bus manager
- DateTime service (RTC abstraction)
- User Preferences service (NVS-backed)
- OLED UI service

### Design Principles
- ESP-IDF native (no Arduino framework)
- FreeRTOS-based
- Thread-safe services
- Clear separation of concerns
- Scalable and production-oriented

---
