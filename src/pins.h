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

#define SD_CS_PIN                   1  // ESP32 pin GPIO1
#define W5500_CS_PIN                19  // ESP32 pin GPIO19
#define MFRC522_CS_PIN              20  // ESP32 pin GPIO20

#define I2C_SDA_PIN                 4  // ESP32 pin GPIO4
#define I2C_SCL_PIN                 5  // ESP32 pin GPIO5

#define SPI_MOSI_PIN                38  // ESP32 pin GPIO38
#define SPI_SCK_PIN                 39  // ESP32 pin GPIO39
#define SPI_MISO_PIN                40  // ESP32 pin GPIO40

// Camera Pins are defined in ov2640_cfg.h

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13
