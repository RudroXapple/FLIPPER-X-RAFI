#pragma once
#include <stdbool.h>

void oled_status_boot_log(const char* line);
void oled_status_show_boot_log(void);
void oled_status_show_system(void);
void oled_status_show_clock(int hour, int minute, int battery_pct, bool wifi_ok);
