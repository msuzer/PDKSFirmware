# ESP32-S3 Access Control System

## Overview
This project is an ESP32-S3–based access control system using RFID authentication, local SD-card logging, and cloud synchronization.
It is designed to operate reliably offline, recover from power loss, and synchronize data when network connectivity is available.

The system is modular and extensible, with planned Ethernet (W5500) and camera support.

## Key Features
- RFID-based access control (MFRC522)
- Local, append-only SD-card logging (FAT32)
- Cloud synchronization over Wi-Fi
- RTC-backed timekeeping (DS3231 + SNTP)
- Offline-first architecture
- FreeRTOS-based, ESP-IDF (no Arduino)

## Hardware
- ESP32-S3 module (N16R8 – 16 MB Flash, 8 MB PSRAM)
- MFRC522 RFID reader (SPI)
- W5500 Ethernet controller (SPI, planned)
- DS3231 RTC (I2C)
- OLED display (I2C)
- SD card (SPI mode)
- Relay, LED, buzzer
- UART logging only

## Software Stack
- ESP-IDF + PlatformIO
- FreeRTOS
- FATFS via esp_vfs_fat
- esp_http_client for cloud communication
- Custom preferences service (NVS)

## Architecture Summary
- Shared SPI bus with manual CS control
- SD card is the source of truth
- Append-only binary logs
- Network layer decoupled from logging logic
- Designed for Wi-Fi and Ethernet coexistence

## Intended Use
- Door/access control
- Audit logging
- Offline-first IoT access systems
- Future extensions: camera capture, Ethernet fallback, remote diagnostics
