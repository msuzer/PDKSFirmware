#include "mfrc522.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pins.h"
#include "services/logger/logger.h"
#include "esp_log.h"

static const char *TAG = "MFRC522";

static uint8_t mfrc522_read_reg(mfrc522_t *dev, uint8_t reg);
static void    mfrc522_write_reg(mfrc522_t *dev, uint8_t reg, uint8_t val);
static bool    mfrc522_transceive(mfrc522_t *dev,
                                  const uint8_t *tx, uint8_t tx_len,
                                  uint8_t *rx, uint8_t *rx_len);

static bool mfrc522_calc_crc(mfrc522_t *dev, const uint8_t *data, uint8_t len, uint8_t out[2])
{
    if (!dev || !data || !out || len == 0)
        return false;

    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);
    mfrc522_write_reg(dev, MFRC522_REG_DIV_IRQ, 0x04);       // Clear CRCIRq
    mfrc522_write_reg(dev, MFRC522_REG_FIFO_LEVEL, 0x80);    // Flush FIFO

    for (uint8_t i = 0; i < len; i++)
        mfrc522_write_reg(dev, MFRC522_REG_FIFO_DATA, data[i]);

    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_CALC_CRC);

    uint16_t timeout = 2000;
    while (timeout--) {
        uint8_t div = mfrc522_read_reg(dev, MFRC522_REG_DIV_IRQ);
        if (div & 0x04) break; // CRCIRq
        vTaskDelay(1);
    }

    out[0] = mfrc522_read_reg(dev, MFRC522_REG_CRC_RESULT_L);
    out[1] = mfrc522_read_reg(dev, MFRC522_REG_CRC_RESULT_H);
    return true;
}



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

    /* 2) Clear IRQ flags and flush FIFO */
    mfrc522_write_reg(dev, MFRC522_REG_COM_IRQ, 0x7F);
    mfrc522_write_reg(dev, MFRC522_REG_FIFO_LEVEL, 0x80);

    /* 3) Write TX data */
    for (uint8_t i = 0; i < tx_len; i++)
        mfrc522_write_reg(dev, MFRC522_REG_FIFO_DATA, tx[i]);

    // ESP_LOGI(TAG, "Transceive start: tx_len=%u first=0x%02X", tx_len, tx[0]);

    /* 4) Start transceive */
    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_TRANSCEIVE);

    /* 5) StartSend = 1 (preserve current bit framing, e.g., 7-bit for REQA) */
    uint8_t bitfr = mfrc522_read_reg(dev, MFRC522_REG_BIT_FRAMING);
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, bitfr | 0x80);

    /* 6) Wait for RX or Idle or timer timeout */
    uint16_t timeout = 50; // ~50ms with 1ms delay
    bool got_irq = false;
    while (timeout--) {
        uint8_t irq = mfrc522_read_reg(dev, MFRC522_REG_COM_IRQ);
        if (irq & 0x30) { // RxIrq(0x20) or IdleIrq(0x10)
            got_irq = true;
            break;
        }
        if (irq & 0x01) { // TimerIrq
            break;
        }
        vTaskDelay(1);
    }

    /* StopSend = 0 (clear StartSend, keep framing bits intact) */
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, bitfr & ~0x80);

    if (!got_irq) {
        // ESP_LOGI(TAG, "Transceive timeout waiting IRQ");
        return false;
    }

    /* 7) Check errors */
    uint8_t err = mfrc522_read_reg(dev, MFRC522_REG_ERROR);
    if (err & 0x13) {    // BufferOvfl | ParityErr | ProtocolErr
        ESP_LOGI(TAG, "Transceive error: ERR=0x%02X", err);
        return false;
    }

    /* 8) Read RX data */
    if (rx && rx_len) {
        uint8_t fifo_len = mfrc522_read_reg(dev, MFRC522_REG_FIFO_LEVEL);
        if (fifo_len > *rx_len)
            fifo_len = *rx_len;

        for (uint8_t i = 0; i < fifo_len; i++)
            rx[i] = mfrc522_read_reg(dev, MFRC522_REG_FIFO_DATA);

        // ESP_LOGI(TAG, "Transceive done: rx_len=%u", fifo_len);
        *rx_len = fifo_len;
    }

    return true;
}

bool mfrc522_is_card_present(mfrc522_t *dev)
{
    if (!dev)
        return false;

    // Reset modes similar to Balboa's PICC_IsNewCardPresent
    mfrc522_write_reg(dev, MFRC522_REG_TX_MODE, 0x00);
    mfrc522_write_reg(dev, MFRC522_REG_RX_MODE, 0x00);
    mfrc522_write_reg(dev, MFRC522_REG_MOD_WIDTH, 0x26);

    uint8_t cmd = PICC_CMD_REQA;
    uint8_t atqa[2];
    uint8_t len = sizeof(atqa);

    // Clear collision handling so bits after collision are cleared
    uint8_t coll = mfrc522_read_reg(dev, MFRC522_REG_COLL);
    mfrc522_write_reg(dev, MFRC522_REG_COLL, coll & ~0x80);

    // REQA must be sent as 7 bits
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, 0x07);

    bool ok = mfrc522_transceive(dev, &cmd, 1, atqa, &len);
    // Restore to 8-bit framing for subsequent operations
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, 0x00);

    if (!ok) {
        // ESP_LOGI(TAG, "REQA failed");
        return false;
    }

    // ESP_LOGI(TAG, "REQA ok: len=%u ATQA=%02X %02X", len, atqa[0], atqa[1]);
    // ATQA must be exactly 2 bytes
    return (len == 2);
}

bool mfrc522_init(mfrc522_t *dev, int cs_gpio)
{
    if (!dev)
        return false;

    ESP_LOGI(TAG, "Init MFRC522 (CS=%d)", cs_gpio);
    if (spi_bus_add_client(&dev->spi,
                           cs_gpio,
                           2 * 1000 * 1000,  // 2 MHz safe default
                           0,                // SPI mode 0
                           false) != ESP_OK)
        return false;

    // Soft reset
    mfrc522_write_reg(dev, MFRC522_REG_COMMAND, MFRC522_CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));

    // Recommended init sequence (aligned with Balboa library)
    mfrc522_write_reg(dev, MFRC522_REG_TX_MODE, 0x00);
    mfrc522_write_reg(dev, MFRC522_REG_RX_MODE, 0x00);
    mfrc522_write_reg(dev, MFRC522_REG_MOD_WIDTH, 0x26);
    mfrc522_write_reg(dev, MFRC522_REG_TMODE, 0x80);       // TAuto=1
    mfrc522_write_reg(dev, MFRC522_REG_TPRESCALER, 0xA9);  // ~40kHz timer
    mfrc522_write_reg(dev, MFRC522_REG_TRELOAD_H, 0x03);   // 25ms timeout
    mfrc522_write_reg(dev, MFRC522_REG_TRELOAD_L, 0xE8);
    mfrc522_write_reg(dev, MFRC522_REG_TX_ASK, 0x40);      // 100% ASK modulation (ISO14443A)
    mfrc522_write_reg(dev, MFRC522_REG_MODE, 0x3D);        // CRC preset 0x6363
    // Antenna on
    uint8_t txc = mfrc522_read_reg(dev, MFRC522_REG_TX_CONTROL);
    mfrc522_write_reg(dev, MFRC522_REG_TX_CONTROL, txc | 0x03);
    ESP_LOGI(TAG, "Init done: TX_CONTROL=0x%02X", mfrc522_read_reg(dev, MFRC522_REG_TX_CONTROL));

    return true;
}

bool mfrc522_read_uid(mfrc522_t *dev, uint8_t *uid, uint8_t *uid_len)
{
    if (!dev || !uid || !uid_len)
        return false;

    // Anticollision CL1: 0x93 0x20
    uint8_t ac_cmd[2] = { PICC_CMD_ANTICOLL_CL1, 0x20 };
    uint8_t resp[10] = {0};
    uint8_t resp_len = sizeof(resp);

    // Ensure 8-bit framing and clear collisions
    mfrc522_write_reg(dev, MFRC522_REG_BIT_FRAMING, 0x00);
    uint8_t coll = mfrc522_read_reg(dev, MFRC522_REG_COLL);
    mfrc522_write_reg(dev, MFRC522_REG_COLL, coll & ~0x80);

    resp_len = 5; // expect 4 UID + 1 BCC
    if (!mfrc522_transceive(dev, ac_cmd, 2, resp, &resp_len)) {
        ESP_LOGD(TAG, "ANTICOLL CL1 failed");
        return false;
    }
    if (resp_len != 5) {
        ESP_LOGD(TAG, "ANTICOLL resp_len=%u", resp_len);
        return false;
    }

    uint8_t bcc = resp[0] ^ resp[1] ^ resp[2] ^ resp[3];
    if (bcc != resp[4]) {
        ESP_LOGD(TAG, "BCC mismatch: calc=%02X got=%02X", bcc, resp[4]);
        return false;
    }

    uid[0] = resp[0]; uid[1] = resp[1]; uid[2] = resp[2]; uid[3] = resp[3];

    // SELECT CL1: 0x93 0x70 + UID[0..3] + BCC + CRC_A
    uint8_t sel[9];
    sel[0] = PICC_CMD_SEL_CL1;
    sel[1] = 0x70;
    sel[2] = uid[0]; sel[3] = uid[1]; sel[4] = uid[2]; sel[5] = uid[3];
    sel[6] = bcc;
    uint8_t crc[2];
    if (!mfrc522_calc_crc(dev, sel, 7, crc))
        return false;
    sel[7] = crc[0]; sel[8] = crc[1];

    uint8_t sak[3] = {0};
    uint8_t sak_len = sizeof(sak);
    if (!mfrc522_transceive(dev, sel, sizeof(sel), sak, &sak_len)) {
        ESP_LOGD(TAG, "SELECT CL1 failed");
        return false;
    }

    // Expect SAK(1) + CRC_A(2)
    if (sak_len < 1) {
        ESP_LOGD(TAG, "SAK too short: %u", sak_len);
        return false;
    }

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
