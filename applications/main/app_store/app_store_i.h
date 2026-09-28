#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <storage/storage.h>
#include <notification/notification.h>

typedef struct AppStore AppStore;

typedef enum {
    AppStoreSceneStart,
    AppStoreSceneMenu,
    AppStoreSceneList,
    AppStoreSceneDetail,
    AppStoreSceneDownload,
    AppStoreSceneCount,
} AppStoreScene;

typedef enum {
    AppStoreCategoryAll = 0,
    AppStoreCategoryGames,
    AppStoreCategoryTools,
    AppStoreCategorySubGHz,
    AppStoreCategoryNFC,
    AppStoreCategoryInfrared,
    AppStoreCategoryMedia,
    AppStoreCategoryMisc,
    AppStoreCategoryCount,
} AppStoreCategory;

#define APP_STORE_MAX_NAME     48
#define APP_STORE_MAX_AUTHOR   32
#define APP_STORE_MAX_URL      256
#define APP_STORE_MAX_APPS     128
#define APP_STORE_MAX_PATH     128

typedef struct {
    char name[APP_STORE_MAX_NAME];
    char author[APP_STORE_MAX_AUTHOR];
    char url[APP_STORE_MAX_URL];
    char category[16];
    uint32_t size;
} AppStoreEntry;

struct AppStore {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Storage* storage;
    NotificationApp* notifications;

    Submenu* submenu;
    Widget* widget;

    AppStoreEntry apps[APP_STORE_MAX_APPS];
    size_t app_count;
    size_t selected_index;
    AppStoreCategory selected_category;

    bool wifi_connected;
    bool download_active;
    uint8_t download_percent;
    char download_message[64];
};
