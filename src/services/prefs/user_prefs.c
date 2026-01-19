#include "user_prefs.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

#include <esp_log.h>
#define TAG "UserPrefs"

#define NVS_NAMESPACE "user_prefs"
#define NVS_KEY       "prefs"

static user_prefs_t prefs;
static nvs_handle_t prefs_nvs_handle;

/* Default preferences */
static void prefs_set_defaults(void) {
    memset(&prefs, 0, sizeof(prefs));

    prefs.version = USER_PREFS_VERSION;
    strcpy(prefs.device_name, "ESP32-Access");
    prefs.door_open_ms = 3000;

    prefs.buzzer_enabled = true;
    prefs.led_enabled = true;
    prefs.oled_brightness = 128;

    prefs.capture_image = true;
    prefs.cloud_upload = true;

    prefs.eth_enabled = false;
    prefs.wifi_enabled = true;
    strcpy(prefs.wifi_ssid, "SUPERONLINE_Wi-Fi_3CF7");
    strcpy(prefs.wifi_pass, "cdN2PKfKtDuz");
}

bool prefs_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &prefs_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed");
        return false;
    }

    size_t size = sizeof(user_prefs_t);
    err = nvs_get_blob(prefs_nvs_handle, NVS_KEY, &prefs, &size);

    if (err != ESP_OK || prefs.version != USER_PREFS_VERSION) {
        ESP_LOGW(TAG, "Prefs invalid or missing, loading defaults");
        prefs_set_defaults();
        prefs_save();
    } else {
        ESP_LOGI(TAG, "Prefs loaded from NVS. version=%u", (unsigned)prefs.version);
    }

    return true;
}

const user_prefs_t *prefs_get(void) {
    return &prefs;
}

bool prefs_set(const user_prefs_t *new_prefs) {
    if (!new_prefs) return false;
    memcpy(&prefs, new_prefs, sizeof(prefs));
    prefs.version = USER_PREFS_VERSION;
    return true;
}

bool prefs_save(void) {
    esp_err_t err = nvs_set_blob(
        prefs_nvs_handle,
        NVS_KEY,
        &prefs,
        sizeof(prefs)
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Prefs save failed");
        return false;
    }

    nvs_commit(prefs_nvs_handle);
    ESP_LOGI(TAG, "Prefs saved");
    return true;
}

void prefs_factory_reset(void) {
    prefs_set_defaults();
    prefs_save();
    ESP_LOGW(TAG, "Prefs factory reset");
}
