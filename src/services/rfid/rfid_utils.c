#include "rfid_utils.h"
#include <stdio.h>

// Internal static buffer for convenience formatter
static char s_uid_buf[40]; // enough for 10 bytes with ':' (29 chars) + null

bool rfid_uid_to_hex(const uint8_t *uid,
                     size_t uid_len,
                     char *out,
                     size_t out_sz,
                     char sep)
{
    if (!uid || !out || uid_len == 0)
        return false;

    // Determine required size
    size_t needed = (sep ? (uid_len * 3 - 1) : (uid_len * 2)) + 1; // +1 for null
    if (out_sz < needed)
        return false;

    char *p = out;
    for (size_t i = 0; i < uid_len; i++) {
        unsigned v = uid[i];
        // Uppercase hex
        p += sprintf(p, "%02X", v);
        if (sep && i + 1 < uid_len) {
            *p++ = sep;
        }
    }
    *p = '\0';
    return true;
}

const char *rfid_uid_to_hex_str(const uint8_t *uid, size_t uid_len)
{
    if (!uid || uid_len == 0 || uid_len > RFID_UID_MAX_LEN) {
        s_uid_buf[0] = '\0';
        return s_uid_buf;
    }
    if (!rfid_uid_to_hex(uid, uid_len, s_uid_buf, sizeof(s_uid_buf), ':')) {
        s_uid_buf[0] = '\0';
    }
    return s_uid_buf;
}
