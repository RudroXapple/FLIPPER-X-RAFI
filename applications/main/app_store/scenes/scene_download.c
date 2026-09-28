#include "../app_store_i.h"
#include "scenes.h"
#include "../helpers/http_fetch.h"
#include "../helpers/wifi_manager.h"
#include <notification/notification_messages.h>

#include <furi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#define TAG "AppStoreDL"

/* Worker state */
static TaskHandle_t dl_task = NULL;
static volatile bool dl_done = false;
static volatile bool dl_ok = false;
static char dl_dest_path[APP_STORE_MAX_PATH];

/* Map category to folder */
static const char* category_to_folder(const char* cat) {
    if(!cat || cat[0] == '\0') return "Misc";
    if(strcasecmp(cat, "Games") == 0) return "Games";
    if(strcasecmp(cat, "Tools") == 0) return "Tools";
    if(strcasecmp(cat, "Sub-GHz") == 0 || strcasecmp(cat, "SubGhz") == 0) return "Sub-GHz";
    if(strcasecmp(cat, "NFC") == 0) return "NFC";
    if(strcasecmp(cat, "Infrared") == 0 || strcasecmp(cat, "IR") == 0) return "Infrared";
    if(strcasecmp(cat, "Media") == 0) return "Media";
    if(strcasecmp(cat, "Bluetooth") == 0 || strcasecmp(cat, "BT") == 0) return "Bluetooth";
    if(strcasecmp(cat, "GPIO") == 0) return "GPIO";
    return "Misc";
}

static void extract_filename(const char* url, char* out, size_t out_size) {
    const char* last_slash = strrchr(url, '/');
    if(last_slash && *(last_slash + 1)) {
        strncpy(out, last_slash + 1, out_size - 1);
        out[out_size - 1] = '\0';
    } else {
        strncpy(out, "app.fap", out_size - 1);
        out[out_size - 1] = '\0';
    }
}

static void build_dest_path(const char* category, const char* url, char* out, size_t out_size) {
    const char* folder = category_to_folder(category);
    char filename[64];
    extract_filename(url, filename, sizeof(filename));
    snprintf(out, out_size, "/ext/apps/%s/%s", folder, filename);
}

/* Worker task — runs download off GUI thread */
static void download_task_fn(void* context) {
    AppStore* app = context;

    FURI_LOG_I(TAG, "Task: downloading...");

    if(!app_store_wifi_is_connected()) {
        FURI_LOG_E(TAG, "WiFi lost");
        dl_ok = false;
        dl_done = true;
        vTaskDelete(NULL);
        return;
    }

    AppStoreEntry* entry = &app->apps[app->selected_index];

    /* Build destination path */
    build_dest_path(entry->category, entry->url, dl_dest_path, sizeof(dl_dest_path));
    FURI_LOG_I(TAG, "URL: %s", entry->url);
    FURI_LOG_I(TAG, "Dest: %s", dl_dest_path);

    /* Create parent directory */
    Storage* storage = furi_record_open(RECORD_STORAGE);
    char parent[APP_STORE_MAX_PATH];
    strncpy(parent, dl_dest_path, sizeof(parent) - 1);
    parent[sizeof(parent) - 1] = '\0';
    char* slash = strrchr(parent, '/');
    if(slash) {
        *slash = '\0';
        storage_common_mkdir(storage, parent);
    }
    furi_record_close(RECORD_STORAGE);

    /* Download */
    bool ok = app_store_http_download_file(entry->url, dl_dest_path, NULL, NULL);

    dl_ok = ok;
    dl_done = true;
    FURI_LOG_I(TAG, "Download result: %s", ok ? "OK" : "FAIL");
    vTaskDelete(NULL);
}

void app_store_scene_download_on_enter(void* context) {
    AppStore* app = context;

    if(app->selected_index >= app->app_count) {
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    dl_done = false;
    dl_ok = false;

    AppStoreEntry* entry = &app->apps[app->selected_index];

    /* Show downloading screen */
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 10, AlignCenter, AlignTop,
                              FontPrimary, "Downloading...");
    widget_add_string_element(app->widget, 64, 28, AlignCenter, AlignTop,
                              FontSecondary, entry->name);
    widget_add_string_element(app->widget, 64, 45, AlignCenter, AlignTop,
                              FontSecondary, "Please wait...");
    view_dispatcher_switch_to_view(app->view_dispatcher, 1);

    if(!app_store_wifi_is_connected()) {
        widget_reset(app->widget);
        widget_add_string_element(app->widget, 64, 25, AlignCenter, AlignTop,
                                  FontPrimary, "WiFi Lost");
        return;
    }

    /* Start download task */
    BaseType_t ret = xTaskCreate(
        download_task_fn,
        "AppDownload",
        12288,
        app,
        5,
        &dl_task);
    if(ret != pdPASS) {
        FURI_LOG_E(TAG, "Task create failed");
        widget_reset(app->widget);
        widget_add_string_element(app->widget, 64, 25, AlignCenter, AlignTop,
                                  FontPrimary, "Task Failed");
    }
}

bool app_store_scene_download_on_event(void* context, SceneManagerEvent event) {
    AppStore* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        if(dl_done) {
            dl_done = false;
            dl_task = NULL;

            AppStoreEntry* entry = &app->apps[app->selected_index];

            widget_reset(app->widget);
            if(dl_ok) {
                widget_add_string_element(app->widget, 64, 15, AlignCenter,
                                          AlignTop, FontPrimary, "Installed!");
                widget_add_string_element(app->widget, 64, 32, AlignCenter,
                                          AlignTop, FontSecondary, entry->name);
                char info[40];
                snprintf(info, sizeof(info), "In: %s",
                         category_to_folder(entry->category));
                widget_add_string_element(app->widget, 64, 48, AlignCenter,
                                          AlignTop, FontSecondary, info);
                notification_message(app->notifications, &sequence_success);
            } else {
                widget_add_string_element(app->widget, 64, 25, AlignCenter,
                                          AlignTop, FontPrimary, "Download Failed");
                widget_add_string_element(app->widget, 64, 42, AlignCenter,
                                          AlignTop, FontSecondary, "Check WiFi/SD");
                notification_message(app->notifications, &sequence_error);
            }
        }
        consumed = true;
    }

    return consumed;
}

void app_store_scene_download_on_exit(void* context) {
    AppStore* app = context;
    if(dl_task) {
        vTaskDelete(dl_task);
        dl_task = NULL;
    }
    widget_reset(app->widget);
}
