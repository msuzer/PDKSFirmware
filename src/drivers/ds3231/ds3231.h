#pragma once
#include <time.h>
#include <stdbool.h>

bool ds3231_init(void);
bool ds3231_get_time(struct tm *timeinfo);
bool ds3231_set_time(const struct tm *timeinfo);
