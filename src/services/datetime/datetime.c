#include "include/project_common.h"

#include "datetime.h"
#include "services/i2c/i2c_bus.h"
#include "drivers/ds3231/ds3231.h"

#include "services/net/net_manager.h" // For network time sync

#include "esp_sntp.h"

#include <esp_log.h>
#define TAG "DateTime"

#define DS3231_ADDR 0x68

static bool s_sntp_started = false;
static bool s_sntp_synced = false;
static SemaphoreHandle_t rtc_mutex;

static void sntp_time_sync_cb(struct timeval *tv);
static void set_system_time_from_rtc(const struct tm *tm);


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

    esp_sntp_set_time_sync_notification_cb(sntp_time_sync_cb);

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

bool datetime_format(time_t ts, char *buffer, size_t len) {
    if (ts == 0) {
        snprintf(buffer, len, "Invalid Time");
        ESP_LOGW(TAG, "Invalid timestamp provided for formatting");
        return false;
    }

    struct tm t;
    gmtime_r(&ts, &t); // format as UTC
    strftime(buffer, len, "%d.%m.%Y %H:%M:%S", &t);
    return true;
}

void show_date_time() {
    char buf[32];
    time_t now = datetime_now();

    if (!datetime_format(now, buf, sizeof(buf))) {
        ESP_LOGE(TAG, "Failed to format date/time");
        return;
    }

    ESP_LOGI(TAG, "Current DateTime: %s", buf);
}

/* Helper to set system time from RTC */
static void set_system_time_from_rtc(const struct tm *tm) {
    time_t t = mktime((struct tm *)tm);
    struct timeval tv = {
        .tv_sec = t,
        .tv_usec = 0
    };
    settimeofday(&tv, NULL);
}

void datetime_request_sntp_sync(void) {
    if (s_sntp_started || s_sntp_synced) {
        return;
    }

    s_sntp_started = true;

    ESP_LOGI(TAG, "Starting SNTP sync");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_init();
}

static void sntp_time_sync_cb(struct timeval *tv) {
    if (!tv) return;

    time_t now = tv->tv_sec;
    struct tm tm_utc = {0};
    gmtime_r(&now, &tm_utc);

    ESP_LOGI(TAG, "SNTP time received");

    // 1) Update system time
    struct timeval sys_tv = {.tv_sec = now, .tv_usec = 0};
    settimeofday(&sys_tv, NULL);

    // 2) Update RTC (this uses ds3231_set_time, which must be I2C-safe)
    datetime_set(&tm_utc);

    s_sntp_synced = true;

    // Optional: stop SNTP to reduce background activity
    esp_sntp_stop();

    ESP_LOGI("DATETIME", "RTC updated from NTP (UTC)");
    show_date_time();
}
