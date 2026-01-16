#include "services/sd/access_log.h"
#include "services/sd/sd_service.h"
#include "services/datetime/datetime.h"
#include "esp_log.h"
#include <string.h>

#define ACCESS_LOG_FILE "/sd/logs/access.bin"

bool access_log_append_rfid(const rfid_event_t *evt, bool granted, bool sent) {
    if (!evt) return false;

    if (!sd_service_is_mounted()) {
        // Avoid heavy work if SD isn't available
        return false;
    }

    // Clamp UID length to prevent overflow
    uint8_t uid_len = evt->uid_len;
    if (uid_len > RFID_UID_MAX_LEN) uid_len = RFID_UID_MAX_LEN;

    access_log_record_t rec;
    memset(&rec, 0, sizeof(rec));

    rec.magic     = ACCESS_LOG_MAGIC;
    rec.version   = ACCESS_LOG_VER;
    rec.size      = sizeof(access_log_record_t);
    rec.unix_time = (uint32_t)datetime_now();

    rec.rfid_uid.uid_len = uid_len;
    memcpy(rec.rfid_uid.uid, evt->uid, uid_len);

    rec.result      = granted ? ACCESS_GRANTED : ACCESS_DENIED;
    rec.via_network = sent ? 1 : 0;
    rec.reserved    = 0;

    return sd_service_write_file(
        "/sd/logs/access.bin",
        &rec,
        sizeof(rec),
        true
    );
}
