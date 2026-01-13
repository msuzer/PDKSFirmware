#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "pins.h"

static TimerHandle_t buzzer_timer;

static void buzzer_off_cb(TimerHandle_t xTimer)
{
    gpio_set_level(USER_BUZZER_PIN, 0);
}

void buzzer_init(void)
{
    gpio_set_direction(USER_BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(USER_BUZZER_PIN, 0);

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
    gpio_set_level(USER_BUZZER_PIN, 1);
    xTimerChangePeriod(buzzer_timer, pdMS_TO_TICKS(duration_ms), 0);
    xTimerStart(buzzer_timer, 0);
}
