#pragma once
#include <stdint.h>
#include <stdbool.h>

/* OLED Display modes */
typedef enum {
    OledModeMirror,   /* Mirror Flipper framebuffer */
    OledModeCustom,   /* Widget/custom rendering */
} OledMode;

/* Set display mode */
void furi_hal_oled_set_mode(OledMode mode);
OledMode furi_hal_oled_get_mode(void);

/* Widget drawing API — only valid in OledModeCustom */
void oled_widget_clear(void);
void oled_widget_draw_str(int x, int y, const char* str);
void oled_widget_draw_str_big(int x, int y, const char* str);
void oled_widget_draw_hline(int x, int y, int w);
void oled_widget_draw_vline(int x, int y, int h);
void oled_widget_draw_rect(int x, int y, int w, int h, bool fill);
void oled_widget_draw_bar(int x, int y, int w, int h, uint8_t pct);
void oled_widget_draw_icon(int x, int y, const uint8_t* icon_1bpp, int icon_w, int icon_h);
void oled_widget_commit(void);
