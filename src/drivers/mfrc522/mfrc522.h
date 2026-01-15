#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"

#include "drivers/spi/spi_bus.h"

/* ================= Registers ================= */
#define MFRC522_REG_COMMAND        0x01
#define MFRC522_REG_COM_IRQ        0x04
#define MFRC522_REG_DIV_IRQ        0x05
#define MFRC522_REG_ERROR          0x06
#define MFRC522_REG_STATUS1        0x07
#define MFRC522_REG_STATUS2        0x08
#define MFRC522_REG_FIFO_DATA      0x09
#define MFRC522_REG_FIFO_LEVEL     0x0A
#define MFRC522_REG_CONTROL        0x0C
#define MFRC522_REG_BIT_FRAMING    0x0D
#define MFRC522_REG_MODE           0x11
#define MFRC522_REG_TX_CONTROL     0x14
#define MFRC522_REG_TX_ASK         0x15
#define MFRC522_REG_CRC_RESULT_L   0x22
#define MFRC522_REG_CRC_RESULT_H   0x21
#define MFRC522_REG_VERSION        0x37

/* ================= Commands ================= */
#define MFRC522_CMD_IDLE           0x00
#define MFRC522_CMD_TRANSCEIVE     0x0C
#define MFRC522_CMD_CALC_CRC       0x03
#define MFRC522_CMD_SOFT_RESET     0x0F

/* ================= PICC ================= */
#define PICC_CMD_REQA              0x26
#define PICC_CMD_ANTICOLL_CL1      0x93
#define PICC_CMD_HALT              0x50

/* ================= API ================= */

/* ========= MFRC522 device handle ========= */
typedef struct {
    spi_client_t spi;   // owned SPI client (from spi_bus)
} mfrc522_t;

/* ========= Lifecycle ========= */
bool    mfrc522_init(mfrc522_t *dev, int cs_gpio);

/* ========= Card operations ========= */
bool    mfrc522_is_card_present(mfrc522_t *dev);
bool    mfrc522_read_uid(mfrc522_t *dev, uint8_t *uid, uint8_t *uid_len);

/* ========= Card state control ========= */
void    mfrc522_halt(mfrc522_t *dev);
void    mfrc522_stop_crypto(mfrc522_t *dev);

/* ========= Debug ========= */
uint8_t mfrc522_read_version(mfrc522_t *dev);