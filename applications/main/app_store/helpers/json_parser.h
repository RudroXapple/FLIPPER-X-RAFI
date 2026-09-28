#pragma once
#include "../app_store_i.h"

size_t app_store_parse_json_apps(const char* json, AppStoreEntry* out, size_t max);
