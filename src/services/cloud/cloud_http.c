#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

#define TAG "CLOUD_HTTP"

/* Change to your test server */
#define CLOUD_URL "http://10.0.0.100:8080/access"

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    // We don’t need to process response body for now
    return ESP_OK;
}

bool cloud_http_post_access(const char *uid_hex,
                            uint32_t unix_time,
                            int result,
                            const char *device)
{
    char body[256];

    int len = snprintf(body, sizeof(body),
        "{"
        "\"uid\":\"%s\","
        "\"timestamp\":%lu,"
        "\"result\":%d,"
        "\"device\":\"%s\""
        "}",
        uid_hex,
        (unsigned long)unix_time,
        result,
        device
    );

    if (len <= 0 || len >= sizeof(body)) {
        ESP_LOGE(TAG, "JSON body too large");
        return false;
    }

    esp_http_client_config_t cfg = {
        .url = CLOUD_URL,
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "http client init failed");
        return false;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, len);

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP status = %d", status);

    esp_http_client_cleanup(client);

    return (status >= 200 && status < 300);
}
