#include "services/net/net_manager.h"
#include "services/cloud/cloud_sync.h"

/* FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* ESP-IDF */
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "nvs_flash.h"

#include <string.h>

/* App */
#include "services/prefs/user_prefs.h"

#include "drivers/w5500/w5500_drv.h"

/* Event bits */
#define NET_CONNECTED_BIT   BIT0
#define NET_WIFI_ACTIVE_BIT BIT1
#define NET_ETH_ACTIVE_BIT  BIT2

static const char *TAG = "net";

static bool s_wifi_netif_created = false;
static esp_netif_t *s_wifi_netif = NULL;
static bool s_wifi_driver_inited = false;

static EventGroupHandle_t s_net_event_group;
static net_if_t s_active_if = NET_IF_NONE;
static bool s_inited = false;
static bool s_started = false;

static void eth_event_handler(void *arg,
                              esp_event_base_t event_base,
                              int32_t event_id,
                              void *event_data)
{
    if (event_base == ETH_EVENT) {
        switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Ethernet Link Up");
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Ethernet Link Down");
            break;
        default:
            break;
        }
    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_ETH_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Ethernet Got IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));
    }
}

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA start -> connect");
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        /* Mark disconnected */
        xEventGroupClearBits(s_net_event_group, NET_CONNECTED_BIT);
        s_active_if = NET_IF_NONE;

        /* Retry */
        ESP_LOGW(TAG, "WiFi disconnected -> retry");
        esp_wifi_connect();
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        /* Connected */
        xEventGroupSetBits(s_net_event_group, NET_CONNECTED_BIT | NET_WIFI_ACTIVE_BIT);
        xEventGroupClearBits(s_net_event_group, NET_ETH_ACTIVE_BIT);
        s_active_if = NET_IF_WIFI;

        cloud_sync_kick();

        return;
    }
}

static bool start_ethernet(const int spi_host, const int cs_pin) {
    if (w5500_drv_init(spi_host, cs_pin) != ESP_OK) {
        ESP_LOGW(TAG, "ETH init failed");
        return false;
    }

    if (!net_manager_wait_connected(20000)) {
        ESP_LOGW(TAG, "ETH no IP, fallback");
        return false;
    }

    return true;
}

static bool start_wifi(void) {
    const user_prefs_t *p = prefs_get();
    if (!p->wifi_enabled) return false;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* handlers already registered */

    wifi_config_t wcfg = {0};
    strncpy((char *)wcfg.sta.ssid, p->wifi_ssid, sizeof(wcfg.sta.ssid));
    strncpy((char *)wcfg.sta.password, p->wifi_pass, sizeof(wcfg.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    return net_manager_wait_connected(10000);
}

bool net_manager_init(void) {
    if (s_inited) return true;

    s_net_event_group = xEventGroupCreate();
    if (!s_net_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        return false;
    }

    /* Note: main.c should call these once globally.
     * We keep init here defensive in case caller forgets.
     */
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "event_loop_create_default failed: %s", esp_err_to_name(err));
        return false;
    }

    if (!s_wifi_netif_created) {
        s_wifi_netif = esp_netif_create_default_wifi_sta();
        if (!s_wifi_netif) {
            ESP_LOGE(TAG, "Failed to create WiFi netif");
            return false;
        }
        s_wifi_netif_created = true;
    }

    if (!s_wifi_driver_inited) {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        s_wifi_driver_inited = true;
    }

    /* Register handlers */
    // Eth events
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_event_handler, NULL));

    // WiFi events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    /* Mode + sane defaults */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    s_inited = true;
    return true;
}

bool net_manager_start(const int spi_host, const int cs_pin) {
    if (!s_inited && !net_manager_init()) return false;
    if (s_started) return true;

    ESP_LOGI(TAG, "Network bring-up: try ETH first");

    if (start_ethernet(spi_host, cs_pin)) {
        ESP_LOGI(TAG, "Using Ethernet");
        s_started = true;
        return true;
    }

    ESP_LOGI(TAG, "Fallback to Wi-Fi");

    if (start_wifi()) {
        ESP_LOGI(TAG, "Using Wi-Fi");
        s_started = true;
        return true;
    }

    ESP_LOGW(TAG, "No network available, offline mode");
    s_active_if = NET_IF_NONE;
    s_started = true;
    return false;
}

bool net_manager_is_connected(void) {
    if (!s_net_event_group) return false;
    EventBits_t b = xEventGroupGetBits(s_net_event_group);
    return (b & NET_CONNECTED_BIT) != 0;
}

net_if_t net_manager_active_if(void) {
    return s_active_if;
}

bool net_manager_wait_connected(uint32_t timeout_ms) {
    if (!s_net_event_group) return false;

    EventBits_t bits = xEventGroupWaitBits(
        s_net_event_group,
        NET_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        pdMS_TO_TICKS(timeout_ms)
    );

    return (bits & NET_CONNECTED_BIT) != 0;
}
