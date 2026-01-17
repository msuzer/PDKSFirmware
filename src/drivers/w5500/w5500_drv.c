#include "w5500_drv.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "driver/spi_master.h"
#include "esp_netif.h"
#include "sdkconfig.h"
#include "esp_eth_mac_spi.h"

static const char *TAG = "w5500";

static esp_eth_handle_t s_eth_handle = NULL;
static esp_netif_t *s_eth_netif = NULL;

#include "esp_event.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "esp_netif.h"

#include "esp_mac.h"   // esp_read_mac

static void set_eth_mac(esp_eth_handle_t eth) {
    uint8_t mac[6];

    // Use ESP32 base MAC for Ethernet (preferred if available in your IDF)
    // If ESP_MAC_ETH not supported in your version, use ESP_MAC_WIFI_STA and tweak locally.
    esp_read_mac(mac, ESP_MAC_ETH);

    // Safety: if still all-zero, derive from Wi-Fi MAC and set locally administered bit
    bool all_zero = true;
    for (int i = 0; i < 6; i++) if (mac[i] != 0) { all_zero = false; break; }
    if (all_zero) {
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        mac[0] |= 0x02;  // locally administered
        mac[0] &= 0xFE;  // unicast
    }

    ESP_ERROR_CHECK(esp_eth_ioctl(eth, ETH_CMD_S_MAC_ADDR, mac));
    ESP_LOGI("w5500", "ETH MAC set to %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

esp_err_t w5500_drv_init(const int spi_host, const int cs_pin) {
    esp_err_t ret;

    /* 1. Create Ethernet netif */
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    s_eth_netif = esp_netif_new(&netif_cfg);
    if (!s_eth_netif) {
        ESP_LOGE(TAG, "Failed to create netif");
        return ESP_FAIL;
    }

    /* 2. SPI device config (CS handled by SPI driver) */
    spi_device_interface_config_t spi_dev_cfg = {
        .command_bits = 16,
        .address_bits = 8,
        .mode = 0,
        .clock_speed_hz = 10 * 1000 * 1000,   // start conservative
        .spics_io_num = cs_pin,
        .queue_size = 20,
    };

    /* 3. W5500 specific config */
    eth_w5500_config_t w5500_config = {
        .spi_host_id = spi_host,
        .spi_devcfg = &spi_dev_cfg,
        .int_gpio_num = -1,        // no INT pin
        .poll_period_ms = 100,     // 🔴 REQUIRED when INT is not used
    };

    /* 4. MAC + PHY config */
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    /* 5. Create MAC + PHY */
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    if (!mac) {
        ESP_LOGE(TAG, "Failed to create W5500 MAC");
        return ESP_FAIL;
    }

    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
    if (!phy) {
        ESP_LOGE(TAG, "Failed to create W5500 PHY");
        return ESP_FAIL;
    }

    /* 6. Install Ethernet driver */
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ret = esp_eth_driver_install(&eth_config, &s_eth_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_eth_driver_install failed");
        return ret;
    }

    /* 7. Attach netif */
    ret = esp_netif_attach(s_eth_netif, esp_eth_new_netif_glue(s_eth_handle));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_attach failed");
        return ret;
    }

    /* Set MAC address */
    set_eth_mac(s_eth_handle);

    /* 8. Start Ethernet */
    ret = esp_eth_start(s_eth_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_eth_start failed");
        return ret;
    }

    ESP_LOGI(TAG, "W5500 init OK");
    return ESP_OK;
}
