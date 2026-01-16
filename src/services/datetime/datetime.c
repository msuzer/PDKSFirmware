#include "include/project_common.h"

#include "datetime.h"
#include "services/i2c/i2c_bus.h"
#include "drivers/ds3231/ds3231.h"

#include "services/net/net_manager.h" // For network time sync

#include "esp_sntp.h"

#include <esp_log.h>
#define TAG "DateTime"

#define DS3231_ADDR 0x68

static bool sntp_started = false;
static SemaphoreHandle_t rtc_mutex;

static void set_system_time_from_rtc(const struct tm *tm);
static void sntp_start(void);
static void sntp_task(void *arg);

bool datetime_init(void) {
    rtc_mutex = xSemaphoreCreateMutex();
    if (!rtc_mutex) return false;

    if (!ds3231_init())
        return false;

    setenv("TZ", "UTC0", 1);
    tzset();

    struct tm rtc_time;
    if (ds3231_get_time(&rtc_time)) {
        set_system_time_from_rtc(&rtc_time);
    }

    // Start SNTP sync in background
    xTaskCreate(sntp_task, "sntp_sync",
                4096, NULL, 4, NULL);

    return true;
}

bool datetime_get(struct tm *timeinfo) {
    if (!timeinfo) return false;

    if (xSemaphoreTake(rtc_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return false;

    bool ok = ds3231_get_time(timeinfo);
    xSemaphoreGive(rtc_mutex);
    return ok;
}


bool datetime_set(const struct tm *timeinfo) {
    if (!timeinfo) return false;

    if (xSemaphoreTake(rtc_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return false;

    bool ok = ds3231_set_time(timeinfo);
    xSemaphoreGive(rtc_mutex);
    return ok;
}


time_t datetime_now(void) {
    struct tm t;
    if (!datetime_get(&t)) return 0;
    return mktime(&t);
}

bool datetime_format(char *buffer, size_t len) {
    struct tm t;
    if (!datetime_get(&t)) {
        snprintf(buffer, len, "Invalid Time");
        ESP_LOGW(TAG, "Failed to get time for formatting");
        return false;
    }

    strftime(buffer, len, "%d-%m-%Y %H:%M:%S", &t);
    return true;
}

void show_date_time() {
    char buf[32];

    if (!datetime_format(buf, sizeof(buf))) {
        ESP_LOGE(TAG, "Failed to format date/time");
        return;
    }

    ESP_LOGI(TAG, "Current DateTime: %s", buf);
}

/* Helper to set system time from RTC */

/* this is a temporary function to set RTC for testing purposes, not needed in production */
/*
static void rtc_set_once_for_test(void) {
    // Example: set to 2026-01-14 02:10:00 (local)
    struct tm t = {0, 8, 2, 14, 0, 2026 - 1900, 0, 0, -1};
    datetime_set(&t);
}*/

static void set_system_time_from_rtc(const struct tm *tm) {
    time_t t = mktime((struct tm *)tm);
    struct timeval tv = {
        .tv_sec = t,
        .tv_usec = 0
    };
    settimeofday(&tv, NULL);
}

static void sntp_start(void) {
    if (sntp_started)
        return;

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_init();

    sntp_started = true;
}

static void sntp_task(void *arg) {
    time_t now = 0;
    struct tm tm = {0};

    if (!net_manager_wait_connected(15000)) {
        ESP_LOGW("DATETIME", "Network not connected; skipping NTP sync");
        vTaskDelete(NULL);
        return;
    }

    sntp_start();

    for (int i = 0; i < 15; i++) {
        time(&now);
        gmtime_r(&now, &tm);   // ✅ use UTC internally

        if (tm.tm_year >= (2026 - 1900)) {
            datetime_set(&tm);  // ✅ sets system time + RTC
            ESP_LOGI("DATETIME", "RTC updated from NTP (UTC)");
            show_date_time();
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    esp_sntp_stop();  // optional, but clean

    vTaskDelete(NULL);
}
