#include "include/project_common.h"
#include "driver/gpio.h"

#include "relay.h"

static TimerHandle_t relay_timer;
static int s_relay_pin = -1;

static void relay_off(TimerHandle_t xTimer)
{
    if (s_relay_pin >= 0) gpio_set_level(s_relay_pin, 0);
}

void relay_init(int relay_pin)
{
    s_relay_pin = relay_pin;
    gpio_set_direction(s_relay_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_relay_pin, 0);

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
    if (s_relay_pin < 0) return;
    gpio_set_level(s_relay_pin, 1);
    xTimerChangePeriod(relay_timer, pdMS_TO_TICKS(duration_ms), 0);
    xTimerStart(relay_timer, 0);
}
