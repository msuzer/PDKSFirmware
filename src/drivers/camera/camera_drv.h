#pragma once

#include "esp_err.h"
#include "services/camera/camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize camera hardware */
esp_err_t camera_drv_init(void);

/* Deinitialize camera hardware */
void camera_drv_deinit(void);

/* Acquire a frame (blocking) */
esp_err_t camera_drv_get_frame(camera_frame_t *out_frame);

/* Return frame buffer to driver */
void camera_drv_return_frame(camera_frame_t *frame);

/* Access underlying sensor (advanced users only) */
sensor_t *camera_drv_get_sensor(void);

#ifdef __cplusplus
}
#endif
