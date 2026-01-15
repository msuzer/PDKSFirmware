#include "ds3231.h"
#include "services/i2c/i2c_bus.h"

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_log.h>
#define TAG "DS3231"

#define DS3231_ADDR 0x68

/* BCD helpers */
static uint8_t bcd_to_dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static esp_err_t ds3231_read(uint8_t reg, uint8_t *data, size_t len)
{
    i2c_port_t port = i2c_bus_get();

    return i2c_master_write_read_device(
        port,
        DS3231_ADDR,
        &reg,
        1,
        data,
        len,
        pdMS_TO_TICKS(100)
    );
}

static esp_err_t ds3231_write(uint8_t reg, const uint8_t *data, size_t len)
{
    i2c_port_t port = i2c_bus_get();

    uint8_t buf[len + 1];
    buf[0] = reg;
    for (size_t i = 0; i < len; i++)
        buf[i + 1] = data[i];

    return i2c_master_write_to_device(
        port,
        DS3231_ADDR,
        buf,
        len + 1,
        pdMS_TO_TICKS(100)
    );
}

bool ds3231_init(void)
{
    uint8_t dummy;
    if (ds3231_read(0x00, &dummy, 1) != ESP_OK) {
        ESP_LOGE(TAG, "DS3231 not responding");
        return false;
    }

    ESP_LOGI(TAG, "DS3231 detected");
    return true;
}

bool ds3231_get_time(struct tm *t)
{
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

bool ds3231_set_time(const struct tm *t)
{
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
