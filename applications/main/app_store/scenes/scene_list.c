#include "../app_store_i.h"
#include "scenes.h"
#include "../helpers/wifi_manager.h"
#include "../helpers/http_fetch.h"
#include "../helpers/json_parser.h"

#include <furi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_heap_caps.h"

#define APPS_JSON_URL \
    "https://raw.githubusercontent.com/RudroXapple/FLIPPER-X-ESP32S3/main/apps.json"

#define APPS_JSON_MAX 65536

/* Worker state */
static TaskHandle_t fetch_task = NULL;
static char* fetch_buf = NULL;
static volatile bool fetch_done = false;
static volatile bool fetch_ok = false;

/* Worker task — xTaskCreate (lwIP sockets cannot run on FuriThread) */
static void fetch_task_fn(void* context) {
    AppStore* app = context;

    FURI_LOG_I("AppStore", "Task: fetching apps.json...");
    furi_delay_ms(1000);

    if(!app_store_wifi_is_connected()) {
        FURI_LOG_E("AppStore", "WiFi lost");
        fetch_ok = false;
        fetch_done = true;
        vTaskDelete(NULL);
        return;
    }

    bool ok = app_store_http_get_text(APPS_JSON_URL, fetch_buf, APPS_JSON_MAX);

    if(ok) {
        app->app_count = app_store_parse_json_apps(fetch_buf, app->apps, APP_STORE_MAX_APPS);
        FURI_LOG_I("AppStore", "Parsed %u apps", (unsigned)app->app_count);
        fetch_ok = (app->app_count > 0);
    } else {
        FURI_LOG_E("AppStore", "Fetch FAILED");
        fetch_ok = false;
    }

    fetch_done = true;
    vTaskDelete(NULL);
}

static void app_store_list_item_cb(void* context, uint32_t index) {
    AppStore* app = context;
    app->selected_index = index;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void show_error(AppStore* app, const char* title, const char* line1, const char* line2) {
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 15, AlignCenter, AlignTop, FontPrimary, title);
    if(line1) widget_add_string_element(app->widget, 64, 30, AlignCenter, AlignTop, FontSecondary, line1);
    if(line2) widget_add_string_element(app->widget, 64, 45, AlignCenter, AlignTop, FontSecondary, line2);
    view_dispatcher_switch_to_view(app->view_dispatcher, 1);
}

void app_store_scene_list_on_enter(void* context) {
    AppStore* app = context;
    fetch_done = false;
    fetch_ok = false;
    app->app_count = 0;

    if(!app->wifi_connected) {
        show_error(app, "No WiFi", "Setup WiFi first", "Settings > WiFi");
        return;
    }

    fetch_buf = heap_caps_malloc(APPS_JSON_MAX, MALLOC_CAP_SPIRAM);
    if(!fetch_buf) fetch_buf = malloc(APPS_JSON_MAX);
    if(!fetch_buf) {
        show_error(app, "Out of Memory", NULL, NULL);
        return;
    }

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Loading...");
    submenu_add_item(app->submenu, "Fetching app list...", 0, NULL, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    /* KEY FIX: xTaskCreate instead of furi_thread_alloc_ex */
    BaseType_t ret = xTaskCreate(
        fetch_task_fn,          /* Task function */
        "AppFetch",             /* Task name */
        12288,                  /* Stack in bytes (12 KB) */
        app,                    /* Parameter */
        5,                      /* Priority */
        &fetch_task);           /* Handle */
    if(ret != pdPASS) {
        FURI_LOG_E("AppStore", "Failed to create fetch task");
        show_error(app, "Task Failed", "Not enough RAM", NULL);
    }
}

bool app_store_scene_list_on_event(void* context, SceneManagerEvent event) {
    AppStore* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_next_scene(app->scene_manager, AppStoreSceneDetail);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        if(!fetch_done) {
            static uint8_t spin_idx = 0;
            static const char spinner[] = "|/-\\";
            char s[32];
            snprintf(s, sizeof(s), "Loading %c  %c", spinner[spin_idx],
                     spinner[(spin_idx + 2) & 3]);
            spin_idx = (spin_idx + 1) & 3;
            submenu_set_header(app->submenu, s);
        }
        if(fetch_done) {
            fetch_done = false;
            fetch_task = NULL;

            if(fetch_ok && app->app_count > 0) {
                submenu_reset(app->submenu);

                /* category filter */
                static const char* cat_names[] = {
                    NULL, "Games", "Tools", "Sub-GHz",
                    "NFC", "Infrared", "Media", "Misc"
                };
                const char* want = NULL;
                if(app->selected_category < AppStoreCategoryCount) {
                    want = cat_names[(int)app->selected_category];
                }

                /* icon per category */
                static const char* cat_icon[] = {
                    "*", "G", "T", "S", "N", "I", "M", "X"
                };
                const char* icon = "*";
                if(app->selected_category < AppStoreCategoryCount)
                    icon = cat_icon[(int)app->selected_category];

                /* Pass 1: count matches */
                size_t shown = 0;
                for(size_t i = 0; i < app->app_count; i++) {
                    if(want && strcasecmp(app->apps[i].category, want) != 0) continue;
                    shown++;
                }

                /* Header with count */
                char header[48];
                if(want)
                    snprintf(header, sizeof(header), "%s %s  %u apps",
                             icon, want, (unsigned)shown);
                else
                    snprintf(header, sizeof(header), "%s All Apps  %u",
                             icon, (unsigned)shown);
                submenu_set_header(app->submenu, header);

                /* Pass 2: add items with prefix */
                for(size_t i = 0; i < app->app_count; i++) {
                    if(want && strcasecmp(app->apps[i].category, want) != 0) continue;
                    char label[64];
                    if(app->apps[i].size > 1024)
                        snprintf(label, sizeof(label), "%s %s (%uK)",
                                 icon, app->apps[i].name,
                                 (unsigned)(app->apps[i].size / 1024));
                    else
                        snprintf(label, sizeof(label), "%s %s",
                                 icon, app->apps[i].name);
                    submenu_add_item(
                        app->submenu, label, i,
                        app_store_list_item_cb, app);
                }
                FURI_LOG_I("AppStore", "Shown %u/%u apps",
                           (unsigned)shown, (unsigned)app->app_count);

                /* Empty category message */
                if(shown == 0) {
                    submenu_add_item(app->submenu, "No apps in this category",
                                     0, NULL, app);
                    submenu_add_item(app->submenu, "Try another section",
                                     0, NULL, app);
                }
                view_dispatcher_switch_to_view(app->view_dispatcher, 0);
            } else {
                char err_buf[64];
                snprintf(err_buf, sizeof(err_buf), "HTTP:%d Err:%d",
                         app_store_last_http_code, app_store_last_esp_err);
                show_error(app, "Fetch Failed", err_buf, "Try again");
            }
        }
        consumed = true;
    }

    return consumed;
}

void app_store_scene_list_on_exit(void* context) {
    AppStore* app = context;
    if(fetch_task) {
        vTaskDelete(fetch_task);
        fetch_task = NULL;
    }
    if(fetch_buf) {
        free(fetch_buf);
        fetch_buf = NULL;
    }
    submenu_reset(app->submenu);
    widget_reset(app->widget);
}
