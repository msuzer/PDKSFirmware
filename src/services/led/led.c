#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

#include "led.h"
#include "pins.h"

static TimerHandle_t led_timer;
static bool led_state;

static void led_cb(TimerHandle_t xTimer)
{
    led_state = !led_state;
    gpio_set_level(USER_LED_PIN, led_state);
}

void led_init(void)
{
    gpio_set_direction(USER_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(USER_LED_PIN, 0);

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
            gpio_set_level(USER_LED_PIN, 0);
            break;
        case LED_ON:
            gpio_set_level(USER_LED_PIN, 1);
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
