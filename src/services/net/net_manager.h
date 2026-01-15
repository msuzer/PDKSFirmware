#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NET_IF_NONE = 0,
    NET_IF_WIFI,
    NET_IF_ETH
} net_if_t;

/* Core lifecycle */
bool net_manager_init(void);
bool net_manager_start(void);

/* Status */
bool net_manager_is_connected(void);
net_if_t net_manager_active_if(void);

/* Synchronization */
bool net_manager_wait_connected(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
