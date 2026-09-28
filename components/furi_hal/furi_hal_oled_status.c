/**
 * OLED Status Screen — displays system info on the secondary SSD1306.
 */

#include "furi_hal_oled_widget.h"
#include "furi_hal_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <stdio.h>
#include <string.h>

#define TAG "OledStatus"

static char g_boot_lines[6][32];
static int g_boot_line_count = 0;
static bool g_status_active = false;

/* Add a boot log line to the ring buffer */
void oled_status_boot_log(const char* line) {
    if(g_boot_line_count < 6) {
        strncpy(g_boot_lines[g_boot_line_count], line, 31);
        g_boot_lines[g_boot_line_count][31] = '\0';
        g_boot_line_count++;
    } else {
        /* Scroll up */
        for(int i = 1; i < 6; i++) {
            strncpy(g_boot_lines[i-1], g_boot_lines[i], 31);
        }
        strncpy(g_boot_lines[5], line, 31);
        g_boot_lines[5][31] = '\0';
    }
}

/* Draw the boot log screen */
void oled_status_show_boot_log(void) {
    furi_hal_oled_set_mode(OledModeCustom);
    oled_widget_clear();

    /* Header */
    oled_widget_draw_str(0, 0, "BOOT LOG");
    oled_widget_draw_hline(0, 9, 128);

    /* Log lines */
    for(int i = 0; i < g_boot_line_count && i < 6; i++) {
        oled_widget_draw_str(0, 12 + i * 9, g_boot_lines[i]);
    }

    oled_widget_commit();
}

/* Draw the system status screen */
void oled_status_show_system(void) {
    furi_hal_oled_set_mode(OledModeCustom);
    oled_widget_clear();

    /* Header with version */
    oled_widget_draw_str(0, 0, "FLIPPER ESP32");
    oled_widget_draw_hline(0, 9, 128);

    /* Free heap info */
    size_t free_heap = xPortGetFreeHeapSize();
    char line[40];
    snprintf(line, sizeof(line), "RAM: %u KB free", (unsigned)(free_heap / 1024));
    oled_widget_draw_str(0, 12, line);

    /* Uptime */
    uint32_t uptime_ms = furi_get_tick() * portTICK_PERIOD_MS;
    uint32_t uptime_sec = uptime_ms / 1000;
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec / 60) % 60;
    snprintf(line, sizeof(line), "UP: %02u:%02u",
             (unsigned)hours, (unsigned)minutes);
    oled_widget_draw_str(0, 23, line);

    /* WiFi status */
    oled_widget_draw_str(0, 34, "WiFi: checking...");

    /* Footer */
    oled_widget_draw_hline(0, 55, 128);
    oled_widget_draw_str(0, 57, "v2.0.0");

    oled_widget_commit();
}

/* Draw the always-on clock screen */
void oled_status_show_clock(int hour, int minute, int battery_pct, bool wifi_ok) {
    furi_hal_oled_set_mode(OledModeCustom);
    oled_widget_clear();

    /* Big clock center */
    char time_buf[8];
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", hour, minute);
    oled_widget_draw_str_big(28, 15, time_buf);

    /* Battery bar at bottom */
    oled_widget_draw_str(0, 42, "BAT");
    oled_widget_draw_bar(30, 42, 98, 8, (uint8_t)battery_pct);

    /* WiFi indicator */
    oled_widget_draw_str(0, 55, wifi_ok ? "WiFi: OK" : "WiFi: --");

    oled_widget_commit();
}
