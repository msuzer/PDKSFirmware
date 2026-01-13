#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "pins.h"

void led_blink_task(void *arg)
{
    gpio_set_direction(USER_LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(USER_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(USER_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
