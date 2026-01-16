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
#define MFRC522_REG_TX_MODE        0x12
#define MFRC522_REG_RX_MODE        0x13
#define MFRC522_REG_TX_CONTROL     0x14
#define MFRC522_REG_TX_ASK         0x15
#define MFRC522_REG_MOD_WIDTH      0x24
#define MFRC522_REG_TMODE          0x2A
#define MFRC522_REG_TPRESCALER     0x2B
#define MFRC522_REG_TRELOAD_H      0x2C
#define MFRC522_REG_TRELOAD_L      0x2D
#define MFRC522_REG_COLL           0x0E
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
#define PICC_CMD_ANTICOLL_CL1      0x93  // Also used as SEL CL1
#define PICC_CMD_SEL_CL1           0x93
#define PICC_CMD_CT                0x88
#define PICC_CMD_HALT              0x50

/* ================= API ================= */

/* ========= Lifecycle ========= */
bool    mfrc522_init(spi_client_t *client, int cs_gpio);

/* ========= Card operations ========= */
bool    mfrc522_is_card_present(void);
bool    mfrc522_read_uid(uint8_t *uid, uint8_t *uid_len);

/* ========= Card state control ========= */
void    mfrc522_halt(void);
void    mfrc522_stop_crypto(void);

/* ========= Debug ========= */
uint8_t mfrc522_read_version(void);