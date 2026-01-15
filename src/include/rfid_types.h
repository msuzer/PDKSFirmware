#pragma once
#include <stdint.h>

// Common RFID event types shared across tasks/services
typedef struct {
    uint8_t uid[10];
    uint8_t uid_len;
} rfid_event_t;
