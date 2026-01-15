#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ds3231.h"
#include "services/i2c/i2c_bus.h"

#include <esp_log.h>
#define TAG "DS3231"

#define DS3231_ADDR 0x68
static i2c_master_dev_handle_t ds3231_dev;

/* BCD helpers */
static uint8_t bcd_to_dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

static esp_err_t ds3231_read(uint8_t reg, uint8_t *data, size_t len) {
    return i2c_bus_txrx(ds3231_dev, &reg, 1, data, len, 100);
}

static esp_err_t ds3231_write(uint8_t reg, const uint8_t *data, size_t len) {
    uint8_t buf[len + 1];
    buf[0] = reg;
    for (size_t i = 0; i < len; i++)
        buf[i + 1] = data[i];
    return i2c_bus_write(ds3231_dev, buf, len + 1, 100);
}

bool ds3231_init(void) {
    // Ensure bus is initialized and add device
    i2c_bus_init();
    ds3231_dev = i2c_bus_add_device(DS3231_ADDR, 400000);

    uint8_t dummy;
    if (ds3231_read(0x00, &dummy, 1) != ESP_OK) {
        ESP_LOGE(TAG, "DS3231 not responding");
        return false;
    }

    ESP_LOGI(TAG, "DS3231 detected");
    return true;
}

bool ds3231_get_time(struct tm *t) {
    if (!t) return false;

    uint8_t buf[7];
    if (ds3231_read(0x00, buf, sizeof(buf)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read time from DS3231");
        return false;
    }

    t->tm_sec  = bcd_to_dec(buf[0] & 0x7F);
    t->tm_min  = bcd_to_dec(buf[1]);
    t->tm_hour = bcd_to_dec(buf[2] & 0x3F);
    t->tm_mday = bcd_to_dec(buf[4]);
    t->tm_mon  = bcd_to_dec(buf[5] & 0x1F) - 1;
    t->tm_year = bcd_to_dec(buf[6]) + 100; // since 1900

    return true;
}

bool ds3231_set_time(const struct tm *t) {
    if (!t) return false;

    uint8_t buf[7];
    buf[0] = dec_to_bcd(t->tm_sec);
    buf[1] = dec_to_bcd(t->tm_min);
    buf[2] = dec_to_bcd(t->tm_hour);
    buf[3] = 0; // day of week (ignored)
    buf[4] = dec_to_bcd(t->tm_mday);
    buf[5] = dec_to_bcd(t->tm_mon + 1);
    buf[6] = dec_to_bcd(t->tm_year - 100);

    return ds3231_write(0x00, buf, sizeof(buf)) == ESP_OK;
}
