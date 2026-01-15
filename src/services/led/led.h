#pragma once
#include <stdint.h>

typedef enum {
    LED_OFF,
    LED_ON,
    LED_BLINK_SLOW,
    LED_BLINK_FAST
} led_mode_t;

void led_init(int led_pin);
void led_set_mode(led_mode_t mode);
