#include "mfrc522.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pins.h"
#include "services/logger/logger.h"

#define MFRC522_CS_LOW()   { gpio_set_level(MFRC522_CS_PIN, 0); esp_rom_delay_us(5);}
#define MFRC522_CS_HIGH()  { gpio_set_level(MFRC522_CS_PIN, 1); esp_rom_delay_us(5);}

static spi_device_handle_t spi;

static uint8_t mfrc522_read_reg(uint8_t reg)
{
    uint8_t tx[] = { (reg << 1) | 0x80, 0x00 };
    uint8_t rx[2];

    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
        .rx_buffer = rx
    };

    MFRC522_CS_LOW();
    spi_device_transmit(spi, &t);
    MFRC522_CS_HIGH();

    return rx[1];
}

static void mfrc522_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t tx[] = { (reg << 1) & 0x7E, val };
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx
    };

    MFRC522_CS_LOW();
    spi_device_transmit(spi, &t);
    MFRC522_CS_HIGH();
}

bool mfrc522_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .sclk_io_num = SPI_SCK_PIN,
        .max_transfer_sz = 64
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1, //MFRC522_CS_PIN,
        .queue_size = 1
    };

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MFRC522_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // CS idle high
    MFRC522_CS_HIGH();

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    vTaskDelay(pdMS_TO_TICKS(50));

    uint8_t version = mfrc522_read_reg(0x37);
    log_info("MFRC522 version: 0x%02X", version);

    return (version == 0x91 || version == 0x92);
}

bool mfrc522_is_card_present(void)
{
    return (mfrc522_read_reg(0x04) & 0x01) == 0;
}

bool mfrc522_read_uid(uint8_t *uid, uint8_t *uid_len)
{
    if (!uid || !uid_len) return false;

    for (int i = 0; i < 4; i++)
        uid[i] = i + 1; // TEMP stub

    *uid_len = 4;
    return true;
}
