#include "services/sd/access_log.h"
#include "services/sd/sd_service.h"
#include "esp_log.h"
#include <stdio.h>

#define ACCESS_LOG_FILE "/sd/logs/access.bin"

static const char *TAG = "ACCESS_LOG";

bool access_log_iterate(access_log_iter_cb_t cb, void *user_ctx) {
    if (!sd_service_is_mounted()) {
        ESP_LOGW(TAG, "SD not mounted");
        return false;
    }

    FILE *f = fopen(ACCESS_LOG_FILE, "rb");
    if (!f) {
        ESP_LOGW(TAG, "No access log file");
        return false;
    }

    access_log_record_t rec;

    while (1) {
        size_t r = fread(&rec, 1, sizeof(rec), f);
        if (r == 0) {
            break;  // EOF
        }

        if (r != sizeof(rec)) {
            ESP_LOGW(TAG, "Partial record, stopping");
            break;
        }

        if (rec.magic != ACCESS_LOG_MAGIC ||
            rec.size != sizeof(rec) ||
            rec.version != ACCESS_LOG_VER) {
            ESP_LOGW(TAG, "Invalid record detected, stopping");
            break;
        }

        if (!cb(&rec, user_ctx)) {
            break;  // user requested stop
        }
    }

    fclose(f);
    return true;
}
