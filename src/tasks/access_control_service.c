#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/rfid/rfid_service.h"
#include "services/relay/relay.h"
#include "services/buzzer/buzzer.h"
#include "services/datetime/datetime.h"
#include "services/logger/logger.h"

void access_control_task(void *arg) {
    QueueHandle_t q = rfid_service_get_queue();
    rfid_event_t evt;

    while (1) {
        if (xQueueReceive(q, &evt, portMAX_DELAY)) {
            printf("RFID UID:");
            for (int i = 0; i < evt.uid_len; i++)
                printf(" %02X", evt.uid[i]);
            printf("\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
