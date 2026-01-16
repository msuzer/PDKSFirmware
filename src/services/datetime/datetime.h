#pragma once
#include <time.h>
#include <stdbool.h>

/* Initialize RTC + service */
bool datetime_init(void);

/* Get current time (thread-safe) */
bool datetime_get(struct tm *timeinfo);

/* Set RTC time */
bool datetime_set(const struct tm *timeinfo);

/* Convenience helpers */
time_t datetime_now(void);
bool datetime_format(time_t ts, char *buffer, size_t len);
void show_date_time();
