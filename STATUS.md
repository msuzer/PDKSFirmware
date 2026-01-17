# Project Status – ESP32-S3 Access Control

## Current Phase
**Feature Hardening & Expansion**
Core bring-up is complete and validated.

## Implemented & Stable
- Build and boot stable
- FreeRTOS running
- I2C bus operational (RTC + OLED)
- Shared SPI bus stable
- Manual CS control implemented
- MFRC522 RFID reader working reliably
- Access decision + relay control integrated
- Preferences service operational
- RTC + SNTP time synchronization working
- SD card driver + FATFS stable
- Append-only binary access logging
- Log reader with integrity scan
- Cloud sync over Wi-Fi verified end-to-end

## Verified Data Flow
RFID → Access decision → SD log → Wi-Fi → HTTP server → ACK → offset update

## Logging Design
- Binary append-only log: /sd/logs/access.bin
- Fixed-size, versioned records
- Power-loss tolerant
- UID stored as raw binary
- Sync progress tracked via /sd/logs/access.sent

## Known Constraints
- SD card must be FAT32
- Manual SPI CS control required
- Wiznet iolibrary intentionally not used
- TLS/authentication deferred

## Next Steps (Ordered)
### 1. Cloud Sync Hardening
- Retry and backoff logic
- Prevent rapid retry loops
- Gate uploads via user preference

### 2. Ethernet Support (W5500)
- Use ESP-IDF native esp_eth
- Reuse existing cloud sync logic
- Implement Wi-Fi ↔ Ethernet failover

### 3. Log Export & Maintenance
- CSV export utility
- Optional HTTP diagnostics endpoint
- Optional log pruning/compaction

### 4. Camera Integration (Later Phase)
- Capture image on granted access
- Store images on SD card
- Link images to log records
- Review memory impact

## Status Summary
Core architecture is complete.
The system is reliable, offline-capable, and cloud-connected.
Future work focuses on robustness and feature expansion.
