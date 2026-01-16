#pragma once
#include <stdbool.h>

bool cloud_sync_init(void);

/* Trigger a sync attempt (non-blocking) */
void cloud_sync_kick(void);
