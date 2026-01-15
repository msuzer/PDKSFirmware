#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* ===== Event type ===== */
typedef struct {
    uint8_t uid[10];
    uint8_t uid_len;
} rfid_event_t;

/* ===== API ===== */
bool        rfid_service_start(int mfrc_cs_pin);
QueueHandle_t rfid_service_get_queue(void);
