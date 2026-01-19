#include "services/sd/access_log.h"
#include "services/sd/sd_service.h"
#include "services/datetime/datetime.h"
#include "esp_log.h"
#include <string.h>

#define ACCESS_LOG_FILE "/sd/logs/access.bin"

bool access_log_append_rfid(const rfid_event_t *evt, bool granted, bool sent, access_log_record_t *out_rec) {
    if (!evt) return false;

    // Clamp UID length to prevent overflow
    uint8_t uid_len = evt->uid_len;
    if (uid_len > RFID_UID_MAX_LEN) uid_len = RFID_UID_MAX_LEN;

    // Build directly into caller-provided buffer when available to avoid memcpy
    access_log_record_t local_rec;
    access_log_record_t *recp = out_rec ? out_rec : &local_rec;
    memset(recp, 0, sizeof(*recp));

    recp->magic     = ACCESS_LOG_MAGIC;
    recp->version   = ACCESS_LOG_VER;
    recp->size      = sizeof(access_log_record_t);
    recp->unix_time = (uint32_t)datetime_now();

    recp->rfid_uid.uid_len = uid_len;
    memcpy(recp->rfid_uid.uid, evt->uid, uid_len);

    recp->result      = granted ? ACCESS_GRANTED : ACCESS_DENIED;
    recp->via_network = sent ? 1 : 0;
    recp->reserved    = 0;

    if (!sd_service_is_mounted()) {
        // SD not available; we still produced the record for the caller
        return false;
    }

    return sd_service_write_file(
        ACCESS_LOG_FILE,
        recp,
        sizeof(*recp),
        true
    );
}
