#pragma once
#include "driver/i2c_master.h"

// Initialize the I2C master bus (new API)
esp_err_t i2c_bus_init(void);

// New API helpers
i2c_master_bus_handle_t i2c_bus_get_bus(void);
i2c_master_dev_handle_t i2c_bus_add_device(uint16_t addr, uint32_t scl_speed_hz);
esp_err_t i2c_bus_txrx(i2c_master_dev_handle_t dev,
					   const uint8_t *tx, size_t tx_len,
					   uint8_t *rx, size_t rx_len,
					   int timeout_ms);
esp_err_t i2c_bus_write(i2c_master_dev_handle_t dev,
						const uint8_t *data, size_t len,
						int timeout_ms);
