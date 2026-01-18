#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

esp_err_t w5500_drv_init(const int spi_host, const int cs_pin);
