#include "camera_drv.h"
#include "ov2640_cfg.h"

#include "esp_camera.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "camera_drv";

esp_err_t camera_drv_init(void) {
    camera_config_t cfg = {
        .pin_pwdn       = CAM_PIN_PWDN,
        .pin_reset      = CAM_PIN_RESET,
        .pin_xclk       = CAM_PIN_XCLK,
        .pin_sccb_sda   = CAM_PIN_SIOD,
        .pin_sccb_scl   = CAM_PIN_SIOC,

        .pin_d0         = CAM_PIN_D0,
        .pin_d1         = CAM_PIN_D1,
        .pin_d2         = CAM_PIN_D2,
        .pin_d3         = CAM_PIN_D3,
        .pin_d4         = CAM_PIN_D4,
        .pin_d5         = CAM_PIN_D5,
        .pin_d6         = CAM_PIN_D6,
        .pin_d7         = CAM_PIN_D7,

        .pin_vsync      = CAM_PIN_VSYNC,
        .pin_href       = CAM_PIN_HREF,
        .pin_pclk       = CAM_PIN_PCLK,

        .xclk_freq_hz   = CAM_XCLK_FREQ_HZ,
        .ledc_timer     = LEDC_TIMER_0,
        .ledc_channel   = LEDC_CHANNEL_0,

        .pixel_format   = CAM_DEFAULT_PIXEL_FORMAT,
        .frame_size     = CAM_DEFAULT_FRAME_SIZE,
        .jpeg_quality   = CAM_DEFAULT_JPEG_QUALITY,
        .fb_count       = CAM_DEFAULT_FB_COUNT,

        .grab_mode      = CAMERA_GRAB_LATEST,
        .fb_location    = CAM_FB_IN_PSRAM ?
                            CAMERA_FB_IN_PSRAM :
                            CAMERA_FB_IN_DRAM
    };

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_camera_init failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Camera initialized");
    return ESP_OK;
}

void camera_drv_deinit(void) {
    esp_camera_deinit();
    ESP_LOGI(TAG, "Camera deinitialized");
}

esp_err_t camera_drv_get_frame(camera_frame_t *out) {
    if (!out) return ESP_ERR_INVALID_ARG;

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Frame capture failed");
        return ESP_FAIL;
    }

    out->data         = fb->buf;
    out->len          = fb->len;
    out->width        = fb->width;
    out->height       = fb->height;
    out->format       = fb->format;
    out->timestamp_us = esp_timer_get_time();
    out->impl         = fb;

    return ESP_OK;
}

void camera_drv_return_frame(camera_frame_t *frame) {
    if (frame && frame->impl) {
        esp_camera_fb_return((camera_fb_t *)frame->impl);
        frame->impl = NULL;
    }
}

sensor_t *camera_drv_get_sensor(void) {
    return esp_camera_sensor_get();
}
