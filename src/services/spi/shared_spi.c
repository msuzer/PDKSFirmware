#include "shared_spi.h"

static SemaphoreHandle_t s_spi_mutex = NULL;

void shared_spi_init(void) {
    if (s_spi_mutex == NULL) {
        s_spi_mutex = xSemaphoreCreateMutex();
    }
}

bool shared_spi_take(TickType_t timeout) {
    if (s_spi_mutex == NULL) shared_spi_init();
    return xSemaphoreTake(s_spi_mutex, timeout) == pdTRUE;
}

void shared_spi_give(void) {
    if (s_spi_mutex) {
        xSemaphoreGive(s_spi_mutex);
    }
}
