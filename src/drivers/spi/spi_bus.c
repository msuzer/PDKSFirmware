#include "spi_bus.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static int _spi_device = -1;
static SemaphoreHandle_t spi_mutex;
static bool bus_ready = false;

esp_err_t spi_bus_init(const int spi_device, const int miso_pin, const int mosi_pin, const int sck_pin) {
    if (bus_ready)
        return ESP_OK;

    spi_bus_config_t buscfg = {
        .miso_io_num = miso_pin,
        .mosi_io_num = mosi_pin,
        .sclk_io_num = sck_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 256,
    };

    _spi_device = spi_device;

    esp_err_t err = spi_bus_initialize(_spi_device, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
        return err;

    spi_mutex = xSemaphoreCreateMutex();
    if (!spi_mutex) {
        spi_bus_free(_spi_device);
        return ESP_ERR_NO_MEM;
    }

    bus_ready = true;
    return ESP_OK;
}

esp_err_t spi_bus_add_client(spi_client_t *client,
                             int cs_gpio,
                             int clock_hz,
                             uint8_t mode,
                             bool half_duplex)
{
    if (!bus_ready || !client)
        return ESP_ERR_INVALID_STATE;

    gpio_set_direction(cs_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(cs_gpio, 1);

    spi_device_interface_config_t cfg = {
        .clock_speed_hz = clock_hz,
        .mode = mode,
        .spics_io_num = -1,   // MANUAL CS
        .queue_size = 1,
        .flags = half_duplex ? SPI_DEVICE_HALFDUPLEX : 0,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(_spi_device, &cfg, &client->dev));
    client->cs_gpio = cs_gpio;

    return ESP_OK;
}

esp_err_t spi_bus_transfer(spi_client_t *client,
                            const void *tx,
                            void *rx,
                            size_t len_bits)
{
    if (!client || !client->dev)
        return ESP_ERR_INVALID_ARG;

    spi_transaction_t t = {
        .length = len_bits,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    xSemaphoreTake(spi_mutex, portMAX_DELAY);

    gpio_set_level(client->cs_gpio, 0);
    esp_err_t err = spi_device_transmit(client->dev, &t);
    gpio_set_level(client->cs_gpio, 1);

    xSemaphoreGive(spi_mutex);
    return err;
}
