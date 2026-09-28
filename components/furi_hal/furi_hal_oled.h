#pragma once
#include <stdint.h>
#include <stdbool.h>

void furi_hal_oled_init(void);
void furi_hal_oled_commit(const uint8_t* data, uint32_t size);
void furi_hal_oled_sleep(void);
void furi_hal_oled_wakeup(void);
bool furi_hal_oled_is_ready(void);
void furi_hal_oled_scan(void);
void furi_hal_oled_scan(void);
