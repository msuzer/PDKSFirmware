#pragma once
#include <stdbool.h>
#include <stdint.h>

#define USER_PREFS_VERSION 1

typedef struct {
    uint8_t version;

    /* System */
    char device_name[32];
    uint32_t door_open_ms;

    /* UI */
    bool buzzer_enabled;
    bool led_enabled;
    uint8_t oled_brightness;

    /* Features */
    bool capture_image;
    bool cloud_upload;

} user_prefs_t;

/* Service API */
bool prefs_init(void);
const user_prefs_t *prefs_get(void);
bool prefs_set(const user_prefs_t *new_prefs);
bool prefs_save(void);
void prefs_factory_reset(void);
