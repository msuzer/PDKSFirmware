#include "services/cloud/cloud_sync.h"
#include "services/sd/access_log.h"
#include "services/sd/sd_service.h"
#include "services/net/net_manager.h"

#include "esp_log.h"
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "CLOUD_SYNC"

#define ACCESS_LOG_FILE "/sd/logs/access.bin"
#define SENT_OFFSET_FILE "/sd/logs/access.sent"

static TaskHandle_t s_sync_task = NULL;

static uint32_t load_sent_offset(void) {
    FILE *f = fopen(SENT_OFFSET_FILE, "rb");
    if (!f) return 0;

    uint32_t off = 0;
    fread(&off, sizeof(off), 1, f);
    fclose(f);
    return off;
}

static void save_sent_offset(uint32_t off) {
    FILE *f = fopen(SENT_OFFSET_FILE, "wb");
    if (!f) return;

    fwrite(&off, sizeof(off), 1, f);
    fflush(f);
    fclose(f);
}

static bool upload_record(const access_log_record_t *rec) {
    // TODO: replace with HTTP POST
    ESP_LOGI(TAG,
        "UPLOAD uid_len=%d result=%d time=%lu",
        rec->rfid_uid.uid_len,
        rec->result,
        (unsigned long)rec->unix_time
    );

    return true; // simulate success
}

static void cloud_sync_task(void *arg) {
    ESP_LOGI(TAG, "Sync task started");

    if (!sd_service_is_mounted()) {
        ESP_LOGW(TAG, "SD not mounted");
        vTaskDelete(NULL);
        return;
    }

    if (!net_manager_is_connected()) {
        ESP_LOGW(TAG, "Network not connected");
        vTaskDelete(NULL);
        return;
    }

    uint32_t sent_off = load_sent_offset();

    FILE *f = fopen(ACCESS_LOG_FILE, "rb");
    if (!f) {
        ESP_LOGW(TAG, "No access log");
        vTaskDelete(NULL);
        return;
    }

    fseek(f, sent_off, SEEK_SET);

    access_log_record_t rec;
    uint32_t cur_off = sent_off;

    while (fread(&rec, sizeof(rec), 1, f) == 1) {

        // basic validation
        if (rec.magic != ACCESS_LOG_MAGIC ||
            rec.size != sizeof(rec)) {
            ESP_LOGW(TAG, "Invalid record, stopping");
            break;
        }

        if (!upload_record(&rec)) {
            ESP_LOGW(TAG, "Upload failed, retry later");
            break;
        }

        cur_off += sizeof(rec);
        save_sent_offset(cur_off);
    }

    fclose(f);
    ESP_LOGI(TAG, "Sync done, offset=%lu", (unsigned long)cur_off);
    vTaskDelete(NULL);
}

bool cloud_sync_init(void) {
    return true;
}

void cloud_sync_kick(void) {
    if (s_sync_task) {
        ESP_LOGI(TAG, "Sync already running");
        return;
    }

    xTaskCreate(
        cloud_sync_task,
        "cloud_sync",
        4096,
        NULL,
        5,
        &s_sync_task
    );
}
