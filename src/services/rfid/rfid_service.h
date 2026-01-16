#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* ===== Event type ===== */
#include "include/rfid_types.h"

/* ===== API ===== */
bool rfid_service_start(const int mfrc_cs_pin);
QueueHandle_t rfid_service_get_queue(void);
