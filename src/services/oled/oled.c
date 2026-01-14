#include "oled.h"
#include "services/logger/logger.h"

void oled_init(void)
{
    log_info("OLED init");
    // I2C + SSD1306 init later
}

void oled_show_message(const char *line1, const char *line2)
{
    log_info("OLED: %s | %s", line1, line2);
}
