#include "oled.h"

#include <esp_log.h>
#define TAG "OLED"

void oled_init(void) {
    ESP_LOGI(TAG, "OLED init");
    // I2C + SSD1306 init later
}

void oled_show_message(const char *line1, const char *line2) {
    ESP_LOGI(TAG, "OLED: %s | %s", line1, line2);
}
