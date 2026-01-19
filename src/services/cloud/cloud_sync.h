#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "services/sd/access_log.h"

/* Trigger a sync attempt (non-blocking) */
void cloud_sync_kick(void);

bool cloud_http_post_access(const char *uid_hex, const char* result, const char* timestamp, const char *device);

/* Send a single access record immediately (synchronous HTTP). */
bool cloud_sync_send_record(const access_log_record_t *rec);
