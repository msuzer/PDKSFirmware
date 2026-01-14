#include "datetime.h"
#include "services/i2c/i2c_bus.h"
#include "services/logger/logger.h"
#include "drivers/ds3231/ds3231.h"
#include "freertos/semphr.h"

#define DS3231_ADDR 0x68

static SemaphoreHandle_t rtc_mutex;

bool datetime_init(void)
{
    rtc_mutex = xSemaphoreCreateMutex();
    if (!rtc_mutex) return false;

    i2c_bus_init();

    if (!ds3231_init()) {
        log_error("RTC init failed");
        return false;
    }

    log_info("DateTime service ready");
    return true;
}

bool datetime_get(struct tm *timeinfo)
{
    if (!timeinfo) return false;

    if (xSemaphoreTake(rtc_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return false;

    bool ok = ds3231_get_time(timeinfo);
    xSemaphoreGive(rtc_mutex);
    return ok;
}


bool datetime_set(const struct tm *timeinfo)
{
    if (!timeinfo) return false;

    if (xSemaphoreTake(rtc_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return false;

    bool ok = ds3231_set_time(timeinfo);
    xSemaphoreGive(rtc_mutex);
    return ok;
}


time_t datetime_now(void)
{
    struct tm t;
    if (!datetime_get(&t)) return 0;
    return mktime(&t);
}

void datetime_format(char *buffer, size_t len)
{
    struct tm t;
    if (!datetime_get(&t)) {
        snprintf(buffer, len, "Invalid Time");
        return;
    }

    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", &t);
}
