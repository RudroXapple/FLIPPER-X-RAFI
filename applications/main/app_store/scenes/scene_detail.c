#include "../app_store_i.h"
#include "scenes.h"

static void app_store_detail_install_cb(void* context, uint32_t index) {
    AppStore* app = context;
    UNUSED(index);
    scene_manager_next_scene(app->scene_manager, AppStoreSceneDownload);
}

static void app_store_detail_back_cb(void* context, uint32_t index) {
    AppStore* app = context;
    UNUSED(index);
    scene_manager_previous_scene(app->scene_manager);
}

void app_store_scene_detail_on_enter(void* context) {
    AppStore* app = context;

    if(app->selected_index >= app->app_count) {
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    AppStoreEntry* entry = &app->apps[app->selected_index];

    /* Use submenu for detail view with Install button */
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, entry->name);

    char info[64];
    snprintf(info, sizeof(info), "Author: %s",
             entry->author[0] ? entry->author : "unknown");
    submenu_add_item(app->submenu, info, 100, NULL, app);

    snprintf(info, sizeof(info), "Category: %s",
             entry->category[0] ? entry->category : "other");
    submenu_add_item(app->submenu, info, 101, NULL, app);

    snprintf(info, sizeof(info), "Size: %lu KB",
             (unsigned long)(entry->size / 1024));
    submenu_add_item(app->submenu, info, 102, NULL, app);

    submenu_add_item(app->submenu, ">> INSTALL <<", 0,
                     app_store_detail_install_cb, app);
    submenu_add_item(app->submenu, "Back", 1,
                     app_store_detail_back_cb, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
}

bool app_store_scene_detail_on_event(void* context, SceneManagerEvent event) {
    AppStore* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void app_store_scene_detail_on_exit(void* context) {
    AppStore* app = context;
    submenu_reset(app->submenu);
}
