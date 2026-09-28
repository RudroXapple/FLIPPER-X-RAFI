#include "json_parser.h"
#include <furi.h>
#include <cJSON.h>
#include <string.h>

#define TAG "AppStoreJSON"

size_t app_store_parse_json_apps(const char* json, AppStoreEntry* out, size_t max) {
    if(!json || !out) return 0;
    cJSON* root = cJSON_Parse(json);
    if(!root) {
        FURI_LOG_E(TAG, "JSON parse failed");
        return 0;
    }

    size_t count = 0;
    cJSON* item;
    cJSON_ArrayForEach(item, root) {
        if(count >= max) break;
        cJSON* name = cJSON_GetObjectItem(item, "name");
        cJSON* author = cJSON_GetObjectItem(item, "author");
        cJSON* url = cJSON_GetObjectItem(item, "url");
        cJSON* cat = cJSON_GetObjectItem(item, "category");
        cJSON* size = cJSON_GetObjectItem(item, "size");

        if(!cJSON_IsString(name) || !cJSON_IsString(url)) continue;

        strncpy(out[count].name, name->valuestring, APP_STORE_MAX_NAME - 1);
        out[count].name[APP_STORE_MAX_NAME - 1] = '\0';

        if(cJSON_IsString(author)) {
            strncpy(out[count].author, author->valuestring, APP_STORE_MAX_AUTHOR - 1);
        } else {
            out[count].author[0] = '\0';
        }

        strncpy(out[count].url, url->valuestring, APP_STORE_MAX_URL - 1);
        out[count].url[APP_STORE_MAX_URL - 1] = '\0';

        if(cJSON_IsString(cat)) {
            strncpy(out[count].category, cat->valuestring, 15);
            out[count].category[15] = '\0';
        }
        if(cJSON_IsNumber(size)) {
            out[count].size = (uint32_t)size->valuedouble;
        }
        count++;
    }
    cJSON_Delete(root);
    FURI_LOG_I(TAG, "Parsed %zu apps", count);
    return count;
}
