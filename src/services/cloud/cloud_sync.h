#pragma once
#include <stdbool.h>
#include <stdint.h>

bool cloud_sync_init(void);

/* Trigger a sync attempt (non-blocking) */
void cloud_sync_kick(void);

bool cloud_http_post_access(const char *uid_hex, const char* result, const char* timestamp, const char *device);
