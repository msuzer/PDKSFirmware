#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_camera.h"
#include "esp_err.h"

/*
 * Camera frame wrapper
 * Ownership:
 *  - impl is owned by driver (camera_fb_t*)
 *  - whoever receives a frame MUST return it
 */
typedef struct {
    const uint8_t *data;
    size_t         len;

    uint16_t       width;
    uint16_t       height;

    pixformat_t    format;
    int64_t        timestamp_us;

    void          *impl;   // driver-private (camera_fb_t*)
} camera_frame_t;

/* Camera operating mode */
typedef enum {
    CAMERA_MODE_IDLE = 0,
    CAMERA_MODE_STREAM,
    CAMERA_MODE_SNAPSHOT,
} camera_mode_t;

/* Camera service state */
typedef enum {
    CAMERA_STATE_STOPPED = 0,
    CAMERA_STATE_RUNNING,
    CAMERA_STATE_ERROR,
} camera_state_t;
