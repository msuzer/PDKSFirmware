#pragma once
#include <stdint.h>

#define RFID_UID_MAX_LEN 10

// Common RFID event types shared across tasks/services
typedef struct {
    uint8_t uid[RFID_UID_MAX_LEN];
    uint8_t uid_len;
} rfid_event_t;
