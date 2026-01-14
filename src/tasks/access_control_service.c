#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/rfid/rfid_service.h"
#include "services/relay/relay.h"
#include "services/buzzer/buzzer.h"
#include "services/datetime/datetime.h"
#include "services/logger/logger.h"

void access_control_task(void *arg)
{
    rfid_event_t evt;

    while (1) {
        if (rfid_get_event(&evt, portMAX_DELAY)) {
            log_info("Card detected, UID length: %d", evt.uid_len);

            // TODO: check UID against authorized list
            bool authorized = true; // TEMP

            if (authorized) {
                log_info("Access granted");
                relay_open(3000);
                buzzer_beep(100);
            } else {
                log_warn("Access denied");
                buzzer_beep(500);
            }
        }
    }
}
