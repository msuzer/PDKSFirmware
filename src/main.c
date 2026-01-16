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
#include "services/net/net_manager.h"

#include "tasks/access_control_service.h"
#include <time.h>

#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "pins.h"

#include <esp_log.h>
#define TAG "Main"

static void rtc_set_once_for_test(void) {
    struct tm t = {0};
    // Example: set to 2026-01-14 02:10:00 (local)
    t.tm_year = 2026 - 1900;
    t.tm_mon  = 0;
    t.tm_mday = 14;
    t.tm_hour = 2;
    t.tm_min  = 8;
    t.tm_sec  = 0;

    datetime_set(&t);
}

void spi_cs_init(void) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    // MFRC522 CS
    io_conf.pin_bit_mask = (1ULL << MFRC522_CS_PIN);
    gpio_config(&io_conf);
    gpio_set_level(MFRC522_CS_PIN, 1);

    // W5500 CS
    io_conf.pin_bit_mask = (1ULL << W5500_CS_PIN);
    gpio_config(&io_conf);
    gpio_set_level(W5500_CS_PIN, 1);

    // Add more SPI devices here (SD, etc.)
}

void app_main(void) {
    spi_cs_init();     // 🔴 FIRST
    prefs_init();
    i2c_bus_init(I2C_SDA_PIN, I2C_SCL_PIN);
    spi_bus_init(SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SCK_PIN);
    led_init(USER_LED_PIN);
    buzzer_init(USER_BUZZER_PIN);
    relay_init(USER_OPEN_DOOR_PIN);
    oled_init();

    esp_netif_init();
    esp_event_loop_create_default();

    net_manager_init();
    net_manager_start();
    datetime_init();

    rfid_service_start(MFRC522_CS_PIN);

    // rtc_set_once_for_test();
    const user_prefs_t *p = prefs_get();

    if (p->led_enabled)
        led_set_mode(LED_BLINK_SLOW);

    if (p->buzzer_enabled)
        buzzer_beep(100);

    oled_show_message(p->device_name, "System Ready");

    show_date_time();

    xTaskCreate(access_control_task, "access_ctrl", 4096, NULL, 6, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
