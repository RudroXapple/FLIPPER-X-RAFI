#pragma once

#include <stdbool.h>

/**
 * Connect to WiFi using saved settings.
 * - Opens RECORD_WIFI
 * - Enables WiFi (auto-reconnect using last saved SSID)
 * - Waits up to 10 seconds for connection
 * @return true if connected
 */
bool app_store_wifi_connect(void);

/** Disconnect WiFi (no-op, keeps connection alive for reuse) */
void app_store_wifi_disconnect(void);

/** Check connection status */
bool app_store_wifi_is_connected(void);

/** Get last saved SSID (may be empty if never configured) */
bool app_store_wifi_get_saved_ssid(char* out, unsigned size);
