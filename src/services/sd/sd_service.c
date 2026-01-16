#include "services/sd/sd_service.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"

#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

static const char *TAG = "SD";

/* Base paths */
#define SD_MOUNT_POINT   "/sd"
#define SD_LOG_DIR       "/sd/logs"
#define SD_SYS_DIR       "/sd/system"

static bool s_sd_mounted = false;
static sdmmc_card_t *s_card = NULL;

static bool path_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

static bool ensure_dir(const char *path) {
    if (path_exists(path))
        return true;

    int r = mkdir(path, 0775);
    if (r == 0)
        return true;

    ESP_LOGE(TAG, "mkdir(%s) failed: errno=%d", path, errno);
    return false;
}

bool sd_service_is_mounted(void) {
    return s_sd_mounted;
}

bool sd_service_unmount(void) {
    if (!s_sd_mounted)
        return true;

    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, s_card);
    s_card = NULL;
    s_sd_mounted = false;

    ESP_LOGI(TAG, "SD unmounted");
    return true;
}

bool sd_service_file_exists(const char *path) {
    return path_exists(path);
}

/* Create required directory structure + base files */
bool sd_service_prepare_fs(void) {
    if (!s_sd_mounted) {
        ESP_LOGW(TAG, "SD not mounted, cannot prepare FS");
        return false;
    }

    if (!ensure_dir(SD_LOG_DIR)) return false;
    if (!ensure_dir(SD_SYS_DIR)) return false;

    /* Create base files if missing (empty files are fine) */
    const char *boot_file = SD_SYS_DIR "/boot.txt";
    if (!path_exists(boot_file)) {
        const char *msg = "boot\n";
        sd_service_write_file(boot_file, msg, strlen(msg), false);
    }

    const char *access_bin = SD_LOG_DIR "/access.bin";
    if (!path_exists(access_bin)) {
        /* create empty */
        sd_service_write_file(access_bin, "", 0, false);
    }

    ESP_LOGI(TAG, "SD folders/files prepared");
    return true;
}

bool sd_service_write_file(const char *path,
                           const void *data,
                           size_t len,
                           bool append)
{
    if (!s_sd_mounted) {
        ESP_LOGW(TAG, "SD not mounted, write skipped");
        return false;
    }

    const char *mode = append ? "ab" : "wb";
    FILE *f = fopen(path, mode);
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return false;
    }

    if (len > 0 && data) {
        size_t w = fwrite(data, 1, len, f);
        if (w != len) {
            ESP_LOGE(TAG, "Short write: %u/%u", (unsigned)w, (unsigned)len);
            fclose(f);
            return false;
        }
    }

    fflush(f);
    fclose(f);
    return true;
}

bool sd_service_init(const int spi_host, const int cs_pin) {
    if (s_sd_mounted) {
        ESP_LOGI(TAG, "Already mounted");
        return true;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = spi_host;  // Use the same SPI host as the client
    // host.max_freq_khz = 400;   // Force init at <= 400 kHz (safe for all cards)

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = cs_pin;
    slot_cfg.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting SD (SPI host=%d, CS=GPIO%d) at %s", host.slot, cs_pin, SD_MOUNT_POINT);

    esp_err_t err = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_cfg, &mount_cfg, &s_card);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(err));
        s_sd_mounted = false;
        s_card = NULL;
        return false;
    }

    s_sd_mounted = true;

    /* Optional: print card info */
    sdmmc_card_print_info(stdout, s_card);

    /* Create folders + base files */
    if (!sd_service_prepare_fs()) {
        ESP_LOGW(TAG, "Mounted, but prepare_fs failed");
        /* Keep mounted; user can retry */
    }

    ESP_LOGI(TAG, "SD mounted OK");
    return true;
}
