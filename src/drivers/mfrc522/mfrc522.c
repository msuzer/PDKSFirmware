#include "mfrc522.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pins.h"
#include "services/logger/logger.h"


static uint8_t mfrc522_read_reg(mfrc522_t *dev, uint8_t reg)
{
    uint8_t tx[2] = { (reg << 1) | 0x80, 0x00 };
    uint8_t rx[2] = {0};

    spi_bus_transfer(&dev->spi, tx, rx, 16);
    return rx[1];
}

static void mfrc522_write_reg(mfrc522_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = { (reg << 1) & 0x7E, val };
    spi_bus_transfer(&dev->spi, tx, NULL, 16);
}

static bool mfrc522_transceive(mfrc522_t *dev,
                               const uint8_t *tx, uint8_t tx_len,
                               uint8_t *rx, uint8_t *rx_len)
{
    if (!dev || !tx || tx_len == 0)
        return false;

    /* 1) Stop any active command */
    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);

    /* 2) Flush FIFO */
    mfrc522_write_reg(dev, MFRC522_REG_FIFO_LEVEL, 0x80);

    /* 3) Write TX data */
    for (uint8_t i = 0; i < tx_len; i++)
        mfrc522_write_reg(dev, MFRC522_REG_FIFO_DATA, tx[i]);

    /* 4) Start transceive */
    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_TRANSCEIVE);

    /* 5) StartSend = 1 (preserve current bit framing, e.g., 7-bit for REQA) */
    uint8_t bitfr = mfrc522_read_reg(dev, MFRC522_REG_BIT_FRAMING);
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, bitfr | 0x80);

    /* 6) Clear irq flags and wait for RX or Idle or timeout */
    mfrc522_write_reg(dev, MFRC522_REG_COM_IRQ, 0x7F); // Clear all irq flags
    uint16_t timeout = 2000;
    bool got_irq = false;
    while (timeout--) {
        uint8_t irq = mfrc522_read_reg(dev, MFRC522_REG_COM_IRQ);
        if (irq & 0x18) { // RxIrq(0x10) or IdleIrq(0x08)
            got_irq = true;
            break;
        }
        vTaskDelay(1);
    }

    /* StopSend = 0 (clear StartSend, keep framing bits intact) */
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, bitfr & ~0x80);

    if (!got_irq) {
        // Timeout waiting for response
        return false;
    }

    /* 7) Check errors */
    uint8_t err = mfrc522_read_reg(dev, MFRC522_REG_ERROR);
    if (err & 0x13)     // BufferOvfl | ParityErr | ProtocolErr
        return false;

    /* 8) Read RX data */
    if (rx && rx_len) {
        uint8_t fifo_len = mfrc522_read_reg(dev, MFRC522_REG_FIFO_LEVEL);
        if (fifo_len > *rx_len)
            fifo_len = *rx_len;

        for (uint8_t i = 0; i < fifo_len; i++)
            rx[i] = mfrc522_read_reg(dev, MFRC522_REG_FIFO_DATA);

        *rx_len = fifo_len;
    }

    return true;
}

bool mfrc522_is_card_present(mfrc522_t *dev)
{
    if (!dev)
        return false;

    uint8_t cmd = PICC_CMD_REQA;
    uint8_t atqa[2];
    uint8_t len = sizeof(atqa);

    // REQA must be sent as 7 bits
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, 0x07);

    bool ok = mfrc522_transceive(dev, &cmd, 1, atqa, &len);
    // Restore to 8-bit framing for subsequent operations
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, 0x00);

    if (!ok)
        return false;

    // ATQA must be exactly 2 bytes
    return (len == 2);
}

bool mfrc522_init(mfrc522_t *dev, int cs_gpio)
{
    if (!dev)
        return false;

    if (spi_bus_add_client(&dev->spi,
                           cs_gpio,
                           2 * 1000 * 1000,  // 2 MHz safe default
                           0,                // SPI mode 0
                           false) != ESP_OK)
        return false;

    // Soft reset
    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));

    // Recommended init sequence
    mfrc522_write_reg(dev, MFRC522_REG_MODE, 0x3D);
    mfrc522_write_reg(dev, MFRC522_REG_TX_ASK, 0x40);      // 100% ASK modulation (ISO14443A)
    mfrc522_write_reg(dev, MFRC522_REG_TX_CONTROL, 0x03);  // Turn antenna drivers on

    return true;
}

bool mfrc522_read_uid(mfrc522_t *dev, uint8_t *uid, uint8_t *uid_len)
{
    if (!uid || !uid_len) return false;

    for (int i = 0; i < 4; i++)
        uid[i] = i + 1; // TEMP stub

    *uid_len = 4;
    return true;
}

uint8_t mfrc522_read_version(mfrc522_t *dev)
{
    if (!dev)
        return 0x00;

    return mfrc522_read_reg(dev, MFRC522_REG_VERSION);
}

void mfrc522_halt(mfrc522_t *dev)
{
    if (!dev)
        return;

    uint8_t cmd[2] = {
        PICC_CMD_HALT,
        0x00
    };

    uint8_t rx_len = 0;
    mfrc522_transceive(dev, cmd, 2, NULL, &rx_len);
    
    // The PICC does not respond to HALT; ignore result
}

void mfrc522_stop_crypto(mfrc522_t *dev)
{
    if (!dev)
        return;

    uint8_t val = mfrc522_read_reg(dev, MFRC522_REG_STATUS2);
    val &= ~(1 << 3); // Clear MFCrypto1On bit
    mfrc522_write_reg(dev, MFRC522_REG_STATUS2, val);
}
