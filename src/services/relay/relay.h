#pragma once
#include <stdint.h>

void relay_init(int relay_pin);
void relay_open(uint32_t duration_ms);
