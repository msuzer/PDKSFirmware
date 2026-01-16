#include "services/net/net_manager.h"

/* FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* ESP-IDF */
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include <string.h>

/* App */
#include "services/prefs/user_prefs.h"

/* Event bits */
#define NET_CONNECTED_BIT   BIT0
#define NET_WIFI_ACTIVE_BIT BIT1
#define NET_ETH_ACTIVE_BIT  BIT2

static const char *TAG = "net";

static EventGroupHandle_t s_net_event_group;
static net_if_t s_active_if = NET_IF_NONE;
static bool s_inited = false;
static bool s_started = false;

static bool credentials_valid(const char *ssid) {
    return (ssid != NULL) && (ssid[0] != '\0');
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
        return;
    }
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

    /* Create default STA netif */
    esp_netif_create_default_wifi_sta();

    /* WiFi init */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return false;
    }

    /* Register handlers */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    /* Mode + sane defaults */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    s_inited = true;
    return true;
}

bool net_manager_start(void) {
    if (!s_inited) {
        if (!net_manager_init()) return false;
    }
    if (s_started) return true;

    /* Load WiFi config */
    const user_prefs_t *p = prefs_get();

    bool enabled = p->wifi_enabled;

    if (!enabled) {
        ESP_LOGW(TAG, "WiFi disabled by config");
        return false;
    }

    wifi_config_t wcfg = {0};
    strncpy((char *)wcfg.sta.ssid, p->wifi_ssid, sizeof(wcfg.sta.ssid));
    strncpy((char *)wcfg.sta.password, p->wifi_pass, sizeof(wcfg.sta.password));

    /* Ensure proper null-termination within bounds */
    wcfg.sta.ssid[sizeof(wcfg.sta.ssid) - 1] = '\0';
    wcfg.sta.password[sizeof(wcfg.sta.password) - 1] = '\0';

    if (!credentials_valid((const char *)wcfg.sta.ssid)) {
        ESP_LOGW(TAG, "WiFi SSID empty; not starting WiFi");
        return false;
    }

    /* Optional hardening */
    wcfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK; /* allow WPA2+; adjust if needed */
    wcfg.sta.pmf_cfg.capable = true;
    wcfg.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_started = true;
    return true;
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
