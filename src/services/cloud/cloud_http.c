#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

#define TAG "CLOUD_HTTP"

/* Change to your test server */
// TODO: make configurable via prefs
#define CLOUD_URL "http://192.168.1.7:8080/access"

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    // Keep the handler lightweight; don't read response here.
    // Response will be read after esp_http_client_perform() returns.
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGW(TAG, "HTTP event: error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP event: connected");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGD(TAG, "HTTP event: headers sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP header: %s: %s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            // Data observed; handled after perform for simplicity.
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP event: finish");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP event: disconnected");
            break;
        default:
            break;
    }
    return ESP_OK;
}

bool cloud_http_post_access(const char *uid_hex,
                            const char* result,
                            const char* timestamp,
                            const char *device)
{
    char body[256];

    int len = snprintf(body, sizeof(body),
        "{"
        "\"uid\":\"%s\","
        "\"result\":\"%s\","
        "\"timestamp\":\"%s\","
        "\"device\":\"%s\""
        "}",
        uid_hex,
        result,
        timestamp,
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

    // Optionally read a small response body safely outside the event handler.
    char resp[128];
    int rlen = esp_http_client_read_response(client, resp, sizeof(resp) - 1);
    if (rlen > 0) {
        resp[rlen] = '\0';
        ESP_LOGI(TAG, "HTTP resp: %s", resp);
    }

    esp_http_client_cleanup(client);

    return (status >= 200 && status < 300);
}
