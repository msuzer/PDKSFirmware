#pragma once

/*
 * OV2640 + ESP32-S3 board configuration
 * Board: Custom ESP32-S3 N16R8
 */

/* ===================== GPIO MAP ===================== */

#define CAM_PIN_PWDN      (-1)
#define CAM_PIN_RESET     (-1)

#define CAM_PIN_XCLK      (15)
#define CAM_PIN_SIOD      (4)
#define CAM_PIN_SIOC      (5)

#define CAM_PIN_D0        (11)  // Y2
#define CAM_PIN_D1        (9)   // Y3
#define CAM_PIN_D2        (8)   // Y4
#define CAM_PIN_D3        (10)  // Y5
#define CAM_PIN_D4        (12)  // Y6
#define CAM_PIN_D5        (18)  // Y7
#define CAM_PIN_D6        (17)  // Y8
#define CAM_PIN_D7        (16)  // Y9

#define CAM_PIN_VSYNC     (6)
#define CAM_PIN_HREF      (7)
#define CAM_PIN_PCLK      (13)

/* ===================== CLOCK ===================== */

// Start conservative; can raise to 20 MHz later
#define CAM_XCLK_FREQ_HZ  (20000000)

/* ===================== DEFAULT FORMAT ===================== */

// Safe bring-up defaults
#define CAM_DEFAULT_PIXEL_FORMAT   PIXFORMAT_JPEG
#define CAM_DEFAULT_FRAME_SIZE     FRAMESIZE_QVGA
#define CAM_DEFAULT_JPEG_QUALITY   (12)
#define CAM_DEFAULT_FB_COUNT       (2)

/* ===================== FRAMEBUFFER ===================== */

#define CAM_FB_IN_PSRAM             (1)
