#pragma once
#include <stdbool.h>
#include "include/project_common.h"

/* Initialize shared SPI mutex (idempotent) */
void shared_spi_init(void);

/* Take shared SPI lock. Returns true on success. */
bool shared_spi_take(TickType_t timeout);

/* Release shared SPI lock. */
void shared_spi_give(void);
