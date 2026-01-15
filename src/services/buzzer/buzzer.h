#pragma once
#include <stdint.h>

void buzzer_init(int buzzer_pin);
void buzzer_beep(uint16_t duration_ms);
