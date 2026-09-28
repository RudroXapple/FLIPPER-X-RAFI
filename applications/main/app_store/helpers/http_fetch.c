#include "http_fetch.h"
#include <furi.h>
#include <storage/storage.h>
#include <esp_http_client.h>
#include <string.h>

#define TAG "AppStoreHTTP"

static void http_cfg(esp_http_client_config_t* cfg, const char* url) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->url = url;
    cfg->timeout_ms = 15000;
    cfg->transport_type = HTTP_TRANSPORT_OVER_SSL;
    cfg->skip_cert_common_name_check = true;
    cfg->crt_bundle_attach = NULL;
    cfg->use_global_ca_store = false;
    cfg->buffer_size = 4096;
    cfg->buffer_size_tx = 1024;
}

bool app_store_http_get_text(const char* url, char* out, size_t out_sz) {
    if(!url || !out || out_sz == 0) return false;
    esp_http_client_config_t cfg;
    http_cfg(&cfg, url);
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if(!client) return false;

    bool ok = false;
    if(esp_http_client_open(client, 0) == ESP_OK) {
        esp_http_client_fetch_headers(client);
        if(esp_http_client_get_status_code(client) == 200) {
            size_t len = 0;
            while(len + 1 < out_sz) {
                int r = esp_http_client_read(client, out + len, out_sz - 1 - len);
                if(r < 0) { len = 0; break; }
                if(r == 0) break;
                len += (size_t)r;
            }
            out[len] = '\0';
            ok = len > 0;
        }
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    FURI_LOG_I(TAG, "GET %s: %s", url, ok ? "OK" : "FAIL");
    return ok;
}

bool app_store_http_download_file(
    const char* url,
    const char* sd_path,
    AppStoreProgressCb progress_cb,
    void* context) {
    if(!url || !sd_path) return false;

    esp_http_client_config_t cfg;
    http_cfg(&cfg, url);
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if(!client) return false;

    bool ok = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(esp_http_client_open(client, 0) == ESP_OK) {
        int content_len = esp_http_client_fetch_headers(client);
        if(esp_http_client_get_status_code(client) == 200) {
            if(storage_file_open(file, sd_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
                char buf[1024];
                size_t total = 0;
                int r;
                ok = true;
                while((r = esp_http_client_read(client, buf, sizeof(buf))) > 0) {
                    if(storage_file_write(file, buf, (size_t)r) != (size_t)r) {
                        ok = false;
                        break;
                    }
                    total += (size_t)r;
                    if(progress_cb && content_len > 0) {
                        progress_cb((uint8_t)((total * 100) / (size_t)content_len), context);
                    }
                }
                storage_file_close(file);
                FURI_LOG_I(TAG, "Downloaded %zu bytes to %s", total, sd_path);
            }
        }
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return ok;
}
