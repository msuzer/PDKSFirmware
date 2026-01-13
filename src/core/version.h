// ============================================
// File: version.h
// Purpose: Defines firmware and device version information
// Part of: Core system info
// License: Proprietary License
// Author: Mehmet H Suzer
// Date: 13 Jan 2026
// ============================================

/*
Specs read from ESP32-S3 chip at runtime:

Chip type: ESP32-S3
Chip Revision: 0
Chip is ESP32-S3 (QFN56) (revision v0.2)
Features: Wi-Fi, BLE, Embedded PSRAM 8MB (AP_3v3)
MAC address: CC:8D:A2:EC:FC:3C
Crystal is 40MHz
Flash ID: 182085
*/

#pragma once

#define FIRMWARE_NAME       "PDKS Firmware"
#define FIRMWARE_AUTHOR     "Mehmet H Suzer"
#define FIRMWARE_VERSION    "1.0.0"
#define DEVICE_VERSION      "HW-Rev-A"

#define BUILD_DATE          __DATE__
#define BUILD_TIME          __TIME__
