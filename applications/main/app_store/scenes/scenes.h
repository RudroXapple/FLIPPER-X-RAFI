#pragma once

#include "../app_store_i.h"

void app_store_scene_start_on_enter(void* context);
bool app_store_scene_start_on_event(void* context, SceneManagerEvent event);
void app_store_scene_start_on_exit(void* context);

void app_store_scene_menu_on_enter(void* context);
bool app_store_scene_menu_on_event(void* context, SceneManagerEvent event);
void app_store_scene_menu_on_exit(void* context);

void app_store_scene_list_on_enter(void* context);
bool app_store_scene_list_on_event(void* context, SceneManagerEvent event);
void app_store_scene_list_on_exit(void* context);

void app_store_scene_detail_on_enter(void* context);
bool app_store_scene_detail_on_event(void* context, SceneManagerEvent event);
void app_store_scene_detail_on_exit(void* context);

void app_store_scene_download_on_enter(void* context);
bool app_store_scene_download_on_event(void* context, SceneManagerEvent event);
void app_store_scene_download_on_exit(void* context);

extern const SceneManagerHandlers app_store_scene_handlers;
