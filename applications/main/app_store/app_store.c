#include "app_store_i.h"
#include "scenes/scenes.h"

#include <furi.h>

/* Custom event callback — routes Submenu OK press to scene manager */
static bool app_store_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    AppStore* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

/* Back event callback — routes Back button to scene manager */
static bool app_store_back_event_callback(void* context) {
    furi_assert(context);
    AppStore* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

/* Tick event callback — forwards tick to scene manager for polling */
static void app_store_tick_callback(void* context) {
    furi_assert(context);
    AppStore* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

/* App allocation */
static AppStore* app_store_alloc(void) {
    AppStore* app = malloc(sizeof(AppStore));
    memset(app, 0, sizeof(AppStore));

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&app_store_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, app_store_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, app_store_back_event_callback);

    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, app_store_tick_callback, 100);

    view_dispatcher_attach_to_gui(
        app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, 0, submenu_get_view(app->submenu));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, 1, widget_get_view(app->widget));

    return app;
}

static void app_store_free(AppStore* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    view_dispatcher_remove_view(app->view_dispatcher, 1);
    submenu_free(app->submenu);
    widget_free(app->widget);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);

    free(app);
}

/* FAP entry point */
int32_t app_store_app(void* p) {
    UNUSED(p);

    AppStore* app = app_store_alloc();

    scene_manager_next_scene(app->scene_manager, AppStoreSceneStart);
    view_dispatcher_run(app->view_dispatcher);

    app_store_free(app);
    return 0;
}
