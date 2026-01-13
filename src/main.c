#include "logger.h"
#include "led.h"
#include "buzzer.h"
#include "oled.h"
#include "datetime.h"
#include "user_prefs.h"
#include "i2c_bus.h"
#include "relay.h"

#include "tasks/led_blink_task.h"
#include <time.h>

static void rtc_set_once_for_test(void)
{
    struct tm t = {0};
    // Example: set to 2026-01-14 02:10:00 (local)
    t.tm_year = 2026 - 1900;
    t.tm_mon  = 0;
    t.tm_mday = 14;
    t.tm_hour = 2;
    t.tm_min  = 8;
    t.tm_sec  = 0;

    datetime_set(&t);
}

void app_main(void)
{
    logger_init();
    prefs_init();
    i2c_bus_init();
    led_init();
    buzzer_init();
    relay_init();
    datetime_init();
    oled_init();

    // rtc_set_once_for_test();
    const user_prefs_t *p = prefs_get();

    if (p->led_enabled)
        led_set_mode(LED_BLINK_SLOW);

    if (p->buzzer_enabled)
        buzzer_beep(100);

    oled_show_message(p->device_name, "System Ready");

    struct tm now;
    if (datetime_get(&now)) {
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &now);
        log_info("RTC time: %s", buf);
    } else {
        log_error("Failed to read RTC");
    }

    xTaskCreate(led_blink_task, "led_blink", 2048, NULL, 5, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
