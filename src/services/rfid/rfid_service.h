#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct {
    uint8_t uid[10];
    uint8_t uid_len;
} rfid_event_t;

void rfid_service_start(void);

/* Consumer API */
bool rfid_get_event(rfid_event_t *event, TickType_t timeout);
