#include "scenes.h"

static void (*const on_enter[])(void*) = {
    app_store_scene_start_on_enter,
    app_store_scene_menu_on_enter,
    app_store_scene_list_on_enter,
    app_store_scene_detail_on_enter,
    app_store_scene_download_on_enter,
};

static bool (*const on_event[])(void*, SceneManagerEvent) = {
    app_store_scene_start_on_event,
    app_store_scene_menu_on_event,
    app_store_scene_list_on_event,
    app_store_scene_detail_on_event,
    app_store_scene_download_on_event,
};

static void (*const scene_on_exit[])(void*) = {
    app_store_scene_start_on_exit,
    app_store_scene_menu_on_exit,
    app_store_scene_list_on_exit,
    app_store_scene_detail_on_exit,
    app_store_scene_download_on_exit,
};

const SceneManagerHandlers app_store_scene_handlers = {
    .on_enter_handlers = on_enter,
    .on_event_handlers = on_event,
    .on_exit_handlers = scene_on_exit,
    .scene_num = AppStoreSceneCount,
};
