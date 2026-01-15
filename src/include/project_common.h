#pragma once

// Centralized common includes for project source files.
// Keep this limited to stable, external headers to avoid wide rebuilds
// when project-specific headers change.

// FreeRTOS core
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

// ESP-IDF logging
#include <esp_log.h>

// ESP-IDF error codes
#include <esp_err.h>
