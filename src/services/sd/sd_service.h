#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "drivers/spi/spi_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Lifecycle */
bool sd_service_init(spi_client_t *client, const int cs_pin);
bool sd_service_is_mounted(void);

/* Directory / file helpers */
bool sd_service_prepare_fs(void);   // creates folders, base files
bool sd_service_file_exists(const char *path);

/* Low-level helpers (used later by logger) */
bool sd_service_write_file(const char *path,
                           const void *data,
                           size_t len,
                           bool append);

#ifdef __cplusplus
}
#endif
