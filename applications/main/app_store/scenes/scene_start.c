#include "../app_store_i.h"
#include "scenes.h"
#include "../helpers/wifi_manager.h"

static void app_store_start_menu_cb(void* context, uint32_t index) {
    AppStore* app = context;
    UNUSED(index);
    scene_manager_next_scene(app->scene_manager, AppStoreSceneMenu);
}

void app_store_scene_start_on_enter(void* context) {
    AppStore* app = context;

    /* Fast WiFi check first */
    app->wifi_connected = app_store_wifi_connect();

    submenu_reset(app->submenu);

    if(app->wifi_connected) {
        submenu_set_header(app->submenu, "App Store - WiFi OK");
        submenu_add_item(app->submenu, "> Browse Apps <", 0,
                         app_store_start_menu_cb, app);
    } else {
        char ssid[40] = {0};
        bool has_ssid = app_store_wifi_get_saved_ssid(ssid, sizeof(ssid));

        submenu_set_header(app->submenu, "WiFi Not Connected");
        if(has_ssid) {
            char label[64];
            snprintf(label, sizeof(label), "SSID: %s (retry)", ssid);
            submenu_add_item(app->submenu, label, 0,
                             app_store_start_menu_cb, app);
        } else {
            submenu_add_item(app->submenu, "Go: Settings > WiFi", 0,
                             app_store_start_menu_cb, app);
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
}

bool app_store_scene_start_on_event(void* context, SceneManagerEvent event) {
    AppStore* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
        consumed = true;
    }

    return consumed;
}

void app_store_scene_start_on_exit(void* context) {
    AppStore* app = context;
    submenu_reset(app->submenu);
}
