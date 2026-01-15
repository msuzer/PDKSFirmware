#include "i2c_bus.h"

#include <esp_log.h>
#define TAG "I2C_BUS"

static bool i2c_inited = false;
static i2c_master_bus_handle_t bus_handle = NULL;

esp_err_t i2c_bus_init(int sda_pin, int scl_pin) {
    if (i2c_inited) {
        return ESP_OK;
    }

    i2c_master_bus_config_t cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .flags = { .enable_internal_pullup = true },
    };

    esp_err_t err = i2c_new_master_bus(&cfg, &bus_handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "I2C master bus initialized");
        i2c_inited = true;
    }
    return err;
}

i2c_master_bus_handle_t i2c_bus_get_bus(void) {
    return bus_handle;
}

i2c_master_dev_handle_t i2c_bus_add_device(uint16_t addr, uint32_t scl_speed_hz) {
    i2c_device_config_t dev_cfg = {
        .device_address = addr,
        .scl_speed_hz = scl_speed_hz,
    };
    i2c_master_dev_handle_t dev;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev));
    return dev;
}

esp_err_t i2c_bus_txrx(i2c_master_dev_handle_t dev,
                       const uint8_t *tx, size_t tx_len,
                       uint8_t *rx, size_t rx_len,
                       int timeout_ms) {
    return i2c_master_transmit_receive(dev, tx, tx_len, rx, rx_len, timeout_ms);
}

esp_err_t i2c_bus_write(i2c_master_dev_handle_t dev,
                        const uint8_t *data, size_t len,
                        int timeout_ms) {
    return i2c_master_transmit(dev, data, len, timeout_ms);
}
