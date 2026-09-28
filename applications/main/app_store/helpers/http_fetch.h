#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*AppStoreProgressCb)(uint8_t percent, void* context);

bool app_store_http_get_text(const char* url, char* out, size_t out_sz);
bool app_store_http_download_file(
    const char* url,
    const char* sd_path,
    AppStoreProgressCb progress_cb,
    void* context);
