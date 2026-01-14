#include "rfid_service.h"
#include "drivers/mfrc522/mfrc522.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "services/logger/logger.h"
#include <string.h>

static QueueHandle_t rfid_queue;

static void rfid_task(void *arg)
{
    uint8_t uid[10];
    uint8_t uid_len;

    while (1) {
        if (mfrc522_is_card_present()) {
            if (mfrc522_read_uid(uid, &uid_len)) {
                rfid_event_t evt = {0};
                evt.uid_len = uid_len;
                memcpy(evt.uid, uid, uid_len);

                xQueueSend(rfid_queue, &evt, 0);

                log_info("RFID UID event queued");
                vTaskDelay(pdMS_TO_TICKS(1000)); // debounce
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void rfid_service_start(void)
{
    if (!mfrc522_init()) {
        log_error("MFRC522 init failed");
        return;
    }

    rfid_queue = xQueueCreate(5, sizeof(rfid_event_t));

    xTaskCreate(
        rfid_task,
        "rfid_task",
        4096,
        NULL,
        5,
        NULL
    );

    log_info("RFID service started");
}

bool rfid_get_event(rfid_event_t *event, TickType_t timeout)
{
    if (!rfid_queue || !event) return false;
    return xQueueReceive(rfid_queue, event, timeout) == pdTRUE;
}
