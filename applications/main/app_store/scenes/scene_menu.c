#include "../app_store_i.h"
#include "scenes.h"

static void app_store_menu_callback(void* context, uint32_t index) {
    AppStore* app = context;
    app->selected_index = index;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void app_store_scene_menu_on_enter(void* context) {
    AppStore* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "App Store");

    submenu_add_item(app->submenu, "All Apps", 0, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "Games", 1, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "Tools", 2, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "Sub-GHz", 3, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "NFC", 4, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "Infrared", 5, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "Media", 6, app_store_menu_callback, app);
    submenu_add_item(app->submenu, "Misc", 7, app_store_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
}

bool app_store_scene_menu_on_event(void* context, SceneManagerEvent event) {
    AppStore* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        /* Go to list scene */
        scene_manager_next_scene(app->scene_manager, AppStoreSceneList);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void app_store_scene_menu_on_exit(void* context) {
    AppStore* app = context;
    submenu_reset(app->submenu);
}
