#include "rfid_service.h"
#include "drivers/mfrc522/mfrc522.h"
#include "drivers/spi/spi_bus.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// #include "services/logger/logger.h"
#include "esp_log.h"
#include <string.h>

#define TAG "RFID"

#define RFID_TASK_STACK   4096
#define RFID_TASK_PRIO    5
#define RFID_QUEUE_LEN   4

static mfrc522_t mfrc;
static QueueHandle_t rfid_queue;

static void rfid_task(void *arg) {
    bool card_was_present = false;
    bool awaiting_absent = false;

    while (1) {
        bool card_present = mfrc522_is_card_present(&mfrc);

        if (card_present != card_was_present) {
            ESP_LOGI(TAG, "card_present=%d", card_present);
        }

        if (card_present && !card_was_present && !awaiting_absent) {

            uint8_t uid[10];
            uint8_t uid_len = 0;

            if (mfrc522_read_uid(&mfrc, uid, &uid_len)) {

                rfid_event_t evt = {0};
                evt.uid_len = uid_len;
                memcpy(evt.uid, uid, uid_len);

                if (xQueueSend(rfid_queue, &evt, 0) == pdTRUE) {
                    ESP_LOGI(TAG, "UID event queued");
                } else {
                    ESP_LOGW(TAG, "RFID queue full");
                }

                mfrc522_halt(&mfrc);
                mfrc522_stop_crypto(&mfrc);

                // Only allow next event after the card is removed
                awaiting_absent = true;
            }
        }

        if (!card_present) {
            card_was_present = false;
            awaiting_absent = false;
        } else {
            card_was_present = true;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

bool rfid_service_start(int mfrc_cs_pin) {
    static bool started = false;
    if (started)
        return true;

    /* Ensure SPI bus is up */
    if (spi_bus_init() != ESP_OK)
        return false;

    /* Init MFRC522 device */
    if (!mfrc522_init(&mfrc, mfrc_cs_pin))
        return false;

    rfid_queue = xQueueCreate(RFID_QUEUE_LEN, sizeof(rfid_event_t));
    if (!rfid_queue)
        return false;

    if (xTaskCreate(rfid_task,
                    "rfid_task",
                    RFID_TASK_STACK,
                    NULL,
                    RFID_TASK_PRIO,
                    NULL) != pdPASS)
        return false;

    started = true;
    ESP_LOGI(TAG, "RFID service started");

    uint8_t v = mfrc522_read_version(&mfrc);
    ESP_LOGI(TAG, "MFRC522 version = 0x%02X", v);

    return true;
}

QueueHandle_t rfid_service_get_queue(void) {
    return rfid_queue;
}
