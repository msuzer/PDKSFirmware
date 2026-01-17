#pragma once

#include "esp_err.h"
#include "esp_netif.h"
#include <stdbool.h>
#include <stdint.h>

esp_err_t w5500_drv_init(const int spi_host, const int cs_pin);
esp_netif_t *w5500_get_netif(void);
