#include "include/project_common.h"

#include "services/rfid/rfid_service.h"
#include "services/rfid/rfid_utils.h"
#include "services/relay/relay.h"
#include "services/buzzer/buzzer.h"
#include "services/datetime/datetime.h"

#include <esp_log.h>
#define TAG "ACCESS_CTRL"

void access_control_task(void *arg) {
    QueueHandle_t q = rfid_service_get_queue();
    rfid_event_t evt;

    while (1) {
        if (xQueueReceive(q, &evt, portMAX_DELAY)) {
            const char *uid_str = rfid_uid_to_hex_str(evt.uid, evt.uid_len);
            ESP_LOGI(TAG, "Card detected, UID: %s", uid_str);

            // TODO: check UID against authorized list
            bool authorized = true; // TEMP

            if (authorized) {
                ESP_LOGI(TAG, "Access granted");
                relay_open(3000);
                buzzer_beep(100);
            } else {
                ESP_LOGW(TAG, "Access denied");
                buzzer_beep(500);
            }
        }
    }
}
