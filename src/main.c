#include <driver/gpio.h>
#include <stdint.h>

#include "drivers/spi/spi_bus.h"

#include "services/led/led.h"
#include "services/buzzer/buzzer.h"
#include "services/oled/oled.h"
#include "services/datetime/datetime.h"
#include "services/prefs/user_prefs.h"
#include "services/i2c/i2c_bus.h"
#include "services/relay/relay.h"
#include "services/rfid/rfid_service.h"
#include "services/rfid/rfid_utils.h"
#include "services/net/net_manager.h"
#include "services/sd/sd_service.h"
#include "services/sd/access_log.h"

#include "tasks/access_control_service.h"
#include <time.h>

#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "pins.h"

#include <esp_log.h>
#define TAG "MAIN"

#define SHARED_SPI_HOST SPI2_HOST

static bool dump_cb(const access_log_record_t *rec, void *ctx) {
    static int record_count = 0;

    record_count++;
    const char *uid_str = rfid_uid_to_hex_str(rec->rfid_uid.uid, rec->rfid_uid.uid_len);

    char ts_buf[32];
    datetime_format(rec->unix_time, ts_buf, sizeof(ts_buf));

    ESP_LOGI(TAG,
        "Log #%d, UID: %s t=%s uid=%s res=%u sent=%u",
        record_count,
        uid_str,
        ts_buf,
        uid_str,
        rec->result,
        rec->via_network
    );
    return true;
}

void spi_cs_init(void) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    // SD-CARD CS
    io_conf.pin_bit_mask = (1ULL << SD_CS_PIN);
    gpio_config(&io_conf);
    gpio_set_level(SD_CS_PIN, 1);

    // MFRC522 CS
    io_conf.pin_bit_mask = (1ULL << MFRC522_CS_PIN);
    gpio_config(&io_conf);
    gpio_set_level(MFRC522_CS_PIN, 1);

    // W5500 CS
    io_conf.pin_bit_mask = (1ULL << W5500_CS_PIN);
    gpio_config(&io_conf);
    gpio_set_level(W5500_CS_PIN, 1);

    // Set pull-up for SPI pins, for sd_card stability
    // gpio_set_pull_mode(SPI_MISO_PIN, GPIO_PULLUP_ONLY);
    // gpio_set_pull_mode(SPI_MOSI_PIN, GPIO_PULLUP_ONLY);

    // Add more SPI devices here (SD, etc.)
}

void app_main(void) {
    spi_cs_init();     // 🔴 FIRST
    prefs_init();
    i2c_bus_init(I2C_SDA_PIN, I2C_SCL_PIN);
    spi_bus_init(SHARED_SPI_HOST, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SCK_PIN);
    led_init(USER_LED_PIN);
    buzzer_init(USER_BUZZER_PIN);
    relay_init(USER_OPEN_DOOR_PIN);
    oled_init();

    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    net_manager_init();
    net_manager_start();
    datetime_init();

    if (!sd_service_init(SHARED_SPI_HOST, SD_CS_PIN)) {
        ESP_LOGW(TAG, "SD not available, continuing without local logs");
    }

    rfid_service_start(MFRC522_CS_PIN);

    // rtc_set_once_for_test();
    const user_prefs_t *p = prefs_get();

    if (p->led_enabled)
        led_set_mode(LED_BLINK_SLOW);

    if (p->buzzer_enabled)
        buzzer_beep(100);

    oled_show_message(p->device_name, "System Ready");

    show_date_time();

    access_log_iterate(dump_cb, NULL);

    xTaskCreate(access_control_task, "access_ctrl", 4096, NULL, 6, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
