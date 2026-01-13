#include "i2c_bus.h"
#include "pins.h"
#include "logger.h"

static i2c_port_t i2c_port = I2C_NUM_0;
static bool i2c_inited = false;


i2c_port_t i2c_bus_init(void)
{
    if (i2c_inited) {
        return i2c_port;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000
    };

    i2c_param_config(i2c_port, &conf);
    i2c_driver_install(i2c_port, conf.mode, 0, 0, 0);

    log_info("I2C bus initialized");
    i2c_inited = true;
    return i2c_port;
}

i2c_port_t i2c_bus_get(void)
{
    return i2c_port;
}
