#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

#include "relay.h"
#include "pins.h"

static TimerHandle_t relay_timer;

static void relay_off(TimerHandle_t xTimer)
{
    gpio_set_level(USER_OPEN_DOOR_PIN, 0);
}

void relay_init(void)
{
    gpio_set_direction(USER_OPEN_DOOR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(USER_OPEN_DOOR_PIN, 0);

    relay_timer = xTimerCreate(
        "relay",
        pdMS_TO_TICKS(1000),
        pdFALSE,
        NULL,
        relay_off
    );
}

void relay_open(uint32_t duration_ms)
{
    gpio_set_level(USER_OPEN_DOOR_PIN, 1);
    xTimerChangePeriod(relay_timer, pdMS_TO_TICKS(duration_ms), 0);
    xTimerStart(relay_timer, 0);
}
