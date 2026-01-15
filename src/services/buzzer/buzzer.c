#include "buzzer.h"
#include "include/project_common.h"
#include "driver/gpio.h"

static TimerHandle_t buzzer_timer;
static int s_buzzer_pin = -1;

static void buzzer_off_cb(TimerHandle_t xTimer)
{
    if (s_buzzer_pin >= 0) gpio_set_level(s_buzzer_pin, 0);
}

void buzzer_init(int buzzer_pin)
{
    s_buzzer_pin = buzzer_pin;
    gpio_set_direction(s_buzzer_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_buzzer_pin, 0);

    buzzer_timer = xTimerCreate(
        "buzzer_timer",
        pdMS_TO_TICKS(100),
        pdFALSE,
        NULL,
        buzzer_off_cb
    );
}

void buzzer_beep(uint16_t duration_ms)
{
    if (s_buzzer_pin < 0) return;
    gpio_set_level(s_buzzer_pin, 1);
    xTimerChangePeriod(buzzer_timer, pdMS_TO_TICKS(duration_ms), 0);
    xTimerStart(buzzer_timer, 0);
}
