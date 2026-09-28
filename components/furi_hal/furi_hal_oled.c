/**
 * SSD1306/SH1106 128x64 I2C display — Secondary display for dual-screen mode.
 * Extracted from furi_hal_display_sh1106.c to work alongside ILI9341.
 */

#include "furi_hal_oled.h"
#include "boards/board.h"

#include <string.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define FB_WIDTH  128U
#define FB_HEIGHT 64U
#define FB_SIZE   (FB_WIDTH * FB_HEIGHT / 8U)
#define I2C_TIMEOUT_TICKS pdMS_TO_TICKS(100)

#if defined(BOARD_HAS_OLED_SSD1306)
static const char* TAG = "FuriHalOLED";
static bool display_ready = false;
static bool panel_asleep = false;

static esp_err_t oled_write(const uint8_t* data, size_t size) {
    return i2c_master_write_to_device(
        BOARD_DISPLAY_I2C_PORT,
        BOARD_DISPLAY_I2C_ADDR,
        data,
        size,
        I2C_TIMEOUT_TICKS);
}

static esp_err_t oled_commands(const uint8_t* commands, size_t count) {
    uint8_t buffer[32];
    if(count + 1U > sizeof(buffer)) return ESP_ERR_INVALID_SIZE;
    buffer[0] = 0x00;
    memcpy(&buffer[1], commands, count);
    return oled_write(buffer, count + 1U);
}

void furi_hal_oled_init(void) {
    ESP_LOGI(TAG, "Initializing OLED at 0x%02X (SDA=%d SCL=%d)",
             BOARD_DISPLAY_I2C_ADDR, BOARD_PIN_DISPLAY_SDA, BOARD_PIN_DISPLAY_SCL);

    const i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BOARD_PIN_DISPLAY_SDA,
        .scl_io_num = BOARD_PIN_DISPLAY_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BOARD_DISPLAY_I2C_FREQ_HZ,
        .clk_flags = 0,
    };
    esp_err_t param_err = i2c_param_config(BOARD_DISPLAY_I2C_PORT, &config);
    if(param_err != ESP_OK && param_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "i2c_param_config failed: %d", param_err);
        return;
    }
    esp_err_t install_err = i2c_driver_install(BOARD_DISPLAY_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
    if(install_err != ESP_OK && install_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %d", install_err);
        return;
    }

#if BOARD_PIN_DISPLAY_RST >= 0
    gpio_set_direction((gpio_num_t)BOARD_PIN_DISPLAY_RST, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)BOARD_PIN_DISPLAY_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level((gpio_num_t)BOARD_PIN_DISPLAY_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
#endif

    /* SSD1306 128x64 page addressing mode init */
    static const uint8_t init_commands[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF,
    };
    if(oled_commands(init_commands, sizeof(init_commands)) != ESP_OK) {
        ESP_LOGE(TAG, "OLED init commands failed");
        return;
    }
    display_ready = true;

    uint8_t clear[FB_SIZE] = {0};
    furi_hal_oled_commit(clear, sizeof(clear));

    ESP_LOGI(TAG, "OLED ready");
}

void furi_hal_oled_commit(const uint8_t* data, uint32_t size) {
    if(!display_ready || !data || size < FB_SIZE) return;

    uint8_t page_data[FB_WIDTH + 1U];
    page_data[0] = 0x40;

    for(uint8_t page = 0; page < 8U; page++) {
        const uint8_t page_commands[] = {(uint8_t)(0xB0U | page), 0x00, 0x10};
        if(oled_commands(page_commands, sizeof(page_commands)) != ESP_OK) return;
        memcpy(&page_data[1], &data[page * FB_WIDTH], FB_WIDTH);
        if(oled_write(page_data, sizeof(page_data)) != ESP_OK) return;
    }
}

void furi_hal_oled_sleep(void) {
    if(!display_ready || panel_asleep) return;
    const uint8_t cmd = 0xAE;
    oled_commands(&cmd, 1);
    panel_asleep = true;
}

void furi_hal_oled_wakeup(void) {
    if(!display_ready || !panel_asleep) return;
    const uint8_t cmd = 0xAF;
    oled_commands(&cmd, 1);
    panel_asleep = false;
}

bool furi_hal_oled_is_ready(void) {
    return display_ready;
}
#else
void furi_hal_oled_init(void) {}
void furi_hal_oled_commit(const uint8_t* data, uint32_t size) { (void)data; (void)size; }
void furi_hal_oled_sleep(void) {}
void furi_hal_oled_wakeup(void) {}
bool furi_hal_oled_is_ready(void) { return false; }
#endif
