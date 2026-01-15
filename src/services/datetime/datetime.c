#include "include/project_common.h"

#include "datetime.h"
#include "services/i2c/i2c_bus.h"
#include "drivers/ds3231/ds3231.h"

#include <esp_log.h>
#define TAG "DateTime"

#define DS3231_ADDR 0x68

static SemaphoreHandle_t rtc_mutex;

bool datetime_init(void)
{
    rtc_mutex = xSemaphoreCreateMutex();
    if (!rtc_mutex) return false;

    // I2C bus must be initialized in main with pin args

    if (!ds3231_init()) {
        ESP_LOGE(TAG, "RTC init failed");
        return false;
    }

    ESP_LOGI(TAG, "DateTime service ready");
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
        ESP_LOGW(TAG, "Failed to get time for formatting");
        return;
    }

    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", &t);
}
