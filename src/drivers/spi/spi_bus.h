#pragma once
#include "driver/spi_master.h"
#include "esp_err.h"

typedef struct {
    spi_device_handle_t dev;
    int cs_gpio;
} spi_client_t;

/* ===== SPI BUS LIFECYCLE ===== */
esp_err_t spi_bus_init(int miso_pin, int mosi_pin, int sck_pin);

/* ===== CLIENT REGISTRATION ===== */
esp_err_t spi_bus_add_client(spi_client_t *client,
                             int cs_gpio,
                             int clock_hz,
                             uint8_t mode,
                             bool half_duplex);

/* ===== TRANSACTION API ===== */
esp_err_t spi_bus_transfer(spi_client_t *client,
                            const void *tx,
                            void *rx,
                            size_t len_bits);
