#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "include/rfid_types.h"

#define ACCESS_LOG_MAGIC  0xACCE5510
#define ACCESS_LOG_VER    1

typedef enum {
    ACCESS_DENIED = 0,
    ACCESS_GRANTED = 1
} access_result_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;         // sanity check
    uint16_t version;       // record version
    uint16_t size;          // sizeof(access_log_record_t)

    uint32_t unix_time;     // UTC, from datetime_now()
    rfid_event_t rfid_uid;  // card UID

    uint8_t  result;        // granted / denied
    uint8_t  via_network;   // 0=offline, 1=cloud sent
    uint16_t img_id;        // 0 if none
    uint16_t reserved;      // future use
} access_log_record_t;

typedef bool (*access_log_iter_cb_t)(
    const access_log_record_t *rec,
    void *user_ctx
);

/* Iterate over all valid log records */
bool access_log_iterate(access_log_iter_cb_t cb, void *user_ctx);

/* Append a new RFID access log record. If out_rec is non-NULL, the
    constructed record is copied out regardless of write success. */
bool access_log_append_rfid(const rfid_event_t *evt, bool granted, bool sent, access_log_record_t *out_rec);
