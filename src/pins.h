/*
pin definitions for ESP32-based project
Notes:
1. Never use strapping pins (GPIO0, GPIO3, GPIO45, GPIO46) for peripherals.
2. GPIO19, GPIO20 are USB Data Pins, do not use them unless USB is not needed.
3. GPIO43, GPIO44 are used for UART0, firmware upload may fail if these pins are used.
*/

#pragma once

#define USER_LED_PIN                48 // ESP32 pin GPIO48
#define USER_BUZZER_PIN             14 // ESP32 pin GPIO14
#define USER_OPEN_DOOR_PIN          21 // ESP32 pin GPIO21

#define W5500_CS_PIN                19  // ESP32 pin GPIO19
#define MFRC522_CS_PIN              20  // ESP32 pin GPIO20

#define I2C_SDA_PIN                 4  // ESP32 pin GPIO4
#define I2C_SCL_PIN                 5  // ESP32 pin GPIO5

#define SPI_MOSI_PIN                38  // ESP32 pin GPIO38
#define SPI_SCK_PIN                 39  // ESP32 pin GPIO39
#define SPI_MISO_PIN                40  // ESP32 pin GPIO40
