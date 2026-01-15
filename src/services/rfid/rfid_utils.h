#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum UID length for MFRC522 is 10 bytes (4/7/10)
#define RFID_UID_MAX_LEN 10

// Formats UID into provided buffer.
// sep='\0' for no separator, otherwise e.g., ':' or '-'.
// Returns true on success, false if out_sz too small or inputs invalid.
bool rfid_uid_to_hex(const uint8_t *uid,
                     size_t uid_len,
                     char *out,
                     size_t out_sz,
                     char sep);

// Convenience helper returning a static buffer. Not thread-safe.
// Uses ':' separators and uppercase hex.
const char *rfid_uid_to_hex_str(const uint8_t *uid, size_t uid_len);

#ifdef __cplusplus
}
#endif
