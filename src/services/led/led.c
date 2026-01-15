#include "include/project_common.h"
#include "driver/gpio.h"

#include "led.h"

static TimerHandle_t led_timer;
static bool led_state;
static int s_led_pin = -1;

static void led_cb(TimerHandle_t xTimer)
{
    led_state = !led_state;
    if (s_led_pin >= 0) gpio_set_level(s_led_pin, led_state);
}

void led_init(int led_pin)
{
    s_led_pin = led_pin;
    gpio_set_direction(s_led_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_led_pin, 0);

    led_timer = xTimerCreate(
        "led",
        pdMS_TO_TICKS(500),
        pdTRUE,
        NULL,
        led_cb
    );
}

void led_set_mode(led_mode_t mode)
{
    xTimerStop(led_timer, 0);

    switch (mode) {
        case LED_OFF:
            if (s_led_pin >= 0) gpio_set_level(s_led_pin, 0);
            break;
        case LED_ON:
            if (s_led_pin >= 0) gpio_set_level(s_led_pin, 1);
            break;
        case LED_BLINK_SLOW:
            xTimerChangePeriod(led_timer, pdMS_TO_TICKS(1000), 0);
            xTimerStart(led_timer, 0);
            break;
        case LED_BLINK_FAST:
            xTimerChangePeriod(led_timer, pdMS_TO_TICKS(200), 0);
            xTimerStart(led_timer, 0);
            break;
    }
}
