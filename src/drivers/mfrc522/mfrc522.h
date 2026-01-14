#pragma once
#include <stdint.h>
#include <stdbool.h>

bool mfrc522_init(void);
bool mfrc522_is_card_present(void);
bool mfrc522_read_uid(uint8_t *uid, uint8_t *uid_len);
