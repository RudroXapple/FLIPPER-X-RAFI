#include "wifi_manager.h"

#include <furi.h>
#include <wifi/wifi.h>
#include <wifi/wlan_hal.h>

#define TAG "AppStoreWiFi"

#define WIFI_FAST_TIMEOUT_MS  3000   /* Only 3 sec max (was 10) */
#define WIFI_POLL_INTERVAL_MS 100

bool app_store_wifi_connect(void) {
    /* Fast path: WiFi already connected? */
    if(wlan_hal_is_connected()) {
        FURI_LOG_I(TAG, "Already connected (fast path)");
        return true;
    }

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    if(!wifi) {
        FURI_LOG_E(TAG, "Failed to open WiFi record");
        return false;
    }

    /* Check if WiFi is globally enabled */
    if(!wifi_is_enabled(wifi)) {
        FURI_LOG_W(TAG, "WiFi disabled — user must enable in Settings");
        furi_record_close(RECORD_WIFI);
        return false;
    }

    /* Check saved SSID */
    WifiSettings settings;
    wifi_get_settings(wifi, &settings);

    if(settings.last_ssid[0] == '\0') {
        FURI_LOG_W(TAG, "No saved SSID — configure in Settings → WiFi");
        furi_record_close(RECORD_WIFI);
        return false;
    }

    FURI_LOG_I(TAG, "Saved SSID: '%s', waiting up to 3s...", settings.last_ssid);

    /* Poll for connection (short timeout) */
    uint32_t waited = 0;
    while(waited < WIFI_FAST_TIMEOUT_MS) {
        if(wifi_is_connected(wifi) || wlan_hal_is_connected()) {
            FURI_LOG_I(TAG, "Connected after %u ms", (unsigned)waited);
            furi_record_close(RECORD_WIFI);
            return true;
        }
        furi_delay_ms(WIFI_POLL_INTERVAL_MS);
        waited += WIFI_POLL_INTERVAL_MS;
    }

    FURI_LOG_E(TAG, "Timeout after %u ms", (unsigned)waited);
    furi_record_close(RECORD_WIFI);
    return false;
}

void app_store_wifi_disconnect(void) {
    /* No-op */
}

bool app_store_wifi_is_connected(void) {
    return wlan_hal_is_connected();
}

bool app_store_wifi_get_saved_ssid(char* out, unsigned size) {
    if(!out || size == 0) return false;

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    if(!wifi) return false;

    WifiSettings settings;
    wifi_get_settings(wifi, &settings);
    furi_record_close(RECORD_WIFI);

    if(settings.last_ssid[0] == '\0') {
        out[0] = '\0';
        return false;
    }

    strncpy(out, settings.last_ssid, size - 1);
    out[size - 1] = '\0';
    return true;
}
