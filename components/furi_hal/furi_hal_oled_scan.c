#include "furi_hal_oled.h"
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#define TAG "OledScan"

void furi_hal_oled_scan(void) {
    ESP_LOGI(TAG, "I2C Bus Scan (SDA=47 SCL=42)...");
    int found = 0;
    for(uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        if(ret == ESP_OK) {
            ESP_LOGI(TAG, "  Found device at 0x%02X", addr);
            found++;
        }
    }
    if(found == 0) {
        ESP_LOGE(TAG, "  No I2C devices found! Check wiring.");
    }
    ESP_LOGI(TAG, "Scan complete. %d device(s) found.", found);
}
