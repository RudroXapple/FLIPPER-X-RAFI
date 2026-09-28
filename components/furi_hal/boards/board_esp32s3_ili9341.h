/**
 * @file board_esp32s3_ili9341.h
 * Board definition: DIY ESP32-S3 + ILI9341 2.8" TFT + 6 Buttons
 *
 * MCU:      ESP32-S3 (dual-core Xtensa LX7)
 * Display:  ILI9341 320x240 RGB565 via SPI (shared bus with SD)
 * Input:    6x Tactile buttons (UP/DOWN/LEFT/RIGHT/OK/BACK)
 * SubGHz:   CC1101 via dedicated SPI (shared with NRF24L01)
 * NRF24:    NRF24L01 via dedicated SPI (shared with CC1101)
 * NFC:      PN532 via I2C
 * SD Card:  SPI (shared bus with LCD)
 * IR:       TX only (no RX)
 * RGB LED:  WS2812 x1
 */

#pragma once

/* ---- Board metadata ---- */
#define BOARD_NAME        "DIY ESP32-S3 ILI9341"
#define BOARD_ID          "esp32s3_ili9341"
#define BOARD_TARGET      "esp32s3"

/* ---- Hardware Button Pins (all active-low, to GND) ---- */
#define BOARD_PIN_BTN_UP        41
#define BOARD_PIN_BTN_DOWN      40
#define BOARD_PIN_BTN_LEFT      38
#define BOARD_PIN_BTN_RIGHT     39
#define BOARD_PIN_BTN_OK        0
#define BOARD_PIN_BTN_BACK      4
#define BOARD_PIN_BUTTON_BOOT   0
#define BOARD_PIN_BATTERY_ADC   UINT16_MAX
/* ---- No rotary encoder ---- */
#define BOARD_PIN_ENCODER_A     UINT16_MAX
#define BOARD_PIN_ENCODER_B     UINT16_MAX
#define BOARD_PIN_ENCODER_BTN   UINT16_MAX
#define BOARD_PIN_BUTTON_KEY    UINT16_MAX

/* ---- LCD Pins (ILI9341 via SPI, shared with SD) ---- */
#define BOARD_PIN_LCD_MOSI      17
#define BOARD_PIN_LCD_SCLK      18
#define BOARD_PIN_LCD_DC        15
#define BOARD_PIN_LCD_CS        7
#define BOARD_PIN_LCD_RST       16
#define BOARD_PIN_LCD_BL        6

/* ---- LCD Display Configuration ---- */
#define BOARD_LCD_H_RES         320
#define BOARD_LCD_V_RES         240
#define BOARD_LCD_SPI_HOST      SPI2_HOST
#define BOARD_LCD_SPI_FREQ_HZ   (40 * 1000 * 1000)
#define BOARD_LCD_CMD_BITS      8
#define BOARD_LCD_PARAM_BITS    8
#define BOARD_LCD_SWAP_XY       true
#define BOARD_LCD_MIRROR_X      false
#define BOARD_LCD_MIRROR_Y      false
#define BOARD_LCD_INVERT_COLOR  false
#define BOARD_LCD_GAP_X         0
#define BOARD_LCD_GAP_Y         0
#define BOARD_LCD_BL_ACTIVE_LOW false
#define BOARD_LCD_COLOR_ORDER_BGR true

/* Flipper framebuffer -> display color mapping */
#define BOARD_LCD_FG_COLOR      0xA0FD
#define BOARD_LCD_FG_COLOR_RB   0x5F03
#define BOARD_LCD_BG_COLOR      0x0000

/* ---- SD Card (shared SPI with LCD) ---- */
#define BOARD_PIN_SD_CS         3
#define BOARD_PIN_SD_MISO       8

/* ---- Touch — NOT PRESENT ---- */
#define BOARD_PIN_TOUCH_SCL     UINT16_MAX
#define BOARD_PIN_TOUCH_SDA     UINT16_MAX
#define BOARD_PIN_TOUCH_RST     UINT16_MAX
#define BOARD_PIN_TOUCH_INT     UINT16_MAX
#define BOARD_TOUCH_I2C_ADDR    0x00
#define BOARD_TOUCH_I2C_PORT    I2C_NUM_0
#define BOARD_TOUCH_I2C_FREQ_HZ 0
#define BOARD_TOUCH_I2C_TIMEOUT 0

/* ---- SubGHz / CC1101 (dedicated SPI, shared with NRF24) ---- */
#define BOARD_PIN_CC1101_SCK    13
#define BOARD_PIN_CC1101_CSN    46
#define BOARD_PIN_CC1101_MISO   11
#define BOARD_PIN_CC1101_MOSI   12
#define BOARD_PIN_CC1101_GDO0   9
#define BOARD_PIN_CC1101_GDO2   10
#define BOARD_PIN_CC1101_SW1    UINT16_MAX
#define BOARD_PIN_CC1101_SW0    UINT16_MAX
#define BOARD_CC1101_SPI_SHARED 0   /* NOT shared with LCD — dedicated SPI bus */

/* ---- NRF24L01 (shares SPI with CC1101) ---- */
#define BOARD_PIN_NRF24_SCK     BOARD_PIN_CC1101_SCK    /* 13 */
#define BOARD_PIN_NRF24_MISO    BOARD_PIN_CC1101_MISO   /* 11 */
#define BOARD_PIN_NRF24_MOSI    BOARD_PIN_CC1101_MOSI   /* 12 */
#define BOARD_PIN_NRF24_CSN     14
#define BOARD_PIN_NRF24_CE      21
#define BOARD_HAS_NRF24         1

/* ---- Power Enable ---- */
#define BOARD_PIN_PWR_EN        UINT16_MAX

/* ---- IR ---- */
#define BOARD_PIN_IR_TX         5
#define BOARD_PIN_IR_RX         1

/* ---- NFC / PN532 (via I2C) ---- */
#define BOARD_PIN_NFC_SCL       42
#define BOARD_PIN_NFC_SDA       47
#define BOARD_PIN_NFC_IRQ       UINT16_MAX
#define BOARD_PIN_NFC_RST       UINT16_MAX
#define BOARD_NFC_I2C_PORT      I2C_NUM_0

/* ---- Speaker (none) ---- */
#define BOARD_PIN_SPEAKER_BCLK  UINT16_MAX
#define BOARD_PIN_SPEAKER_WCLK  UINT16_MAX
#define BOARD_PIN_SPEAKER_DOUT  UINT16_MAX

/* ---- WS2812 RGB LED ---- */
#define BOARD_PIN_WS2812_DATA   48
#define BOARD_WS2812_LED_COUNT  1

/* ---- Microphone (none) ---- */
#define BOARD_PIN_MIC_DATA      UINT16_MAX
#define BOARD_PIN_MIC_CLK       UINT16_MAX

/* ---- Qwiic / External I2C (shared with NFC) ---- */
#define BOARD_PIN_QWIIC_SDA     BOARD_PIN_NFC_SDA
#define BOARD_PIN_QWIIC_SCL     BOARD_PIN_NFC_SCL

/* ---- Features ---- */
#define BOARD_HAS_TOUCH         0
#define BOARD_HAS_ENCODER       0
#define BOARD_HAS_SD_CARD       1
#define BOARD_HAS_BLE           1
#define BOARD_HAS_RGB_LED       1
#define BOARD_HAS_VIBRO         0
#define BOARD_HAS_SPEAKER       0
#define BOARD_HAS_IR            1
#define BOARD_HAS_IBUTTON       0
#define BOARD_HAS_RFID          0
#define BOARD_HAS_NFC           1
#define BOARD_HAS_SUBGHZ        1
#define BOARD_HAS_MIC           0

/* ---- RFID (none) ---- */
#define BOARD_PIN_RFID_RX       UINT16_MAX
#define BOARD_PIN_RFID_TX       UINT16_MAX
#define BOARD_RFID_UART_NUM     -1

/* ---- Power management (no fuel gauge on this board) ---- */
#define BQ27220_ADDR                        0x55
#define BQ_I2C_PORT                         I2C_NUM_0
#define BQ_I2C_SDA                          BOARD_PIN_QWIIC_SDA
#define BQ_I2C_SCL                          BOARD_PIN_QWIIC_SCL
#define HIGH_DRAIN_CURRENT_THRESHOLD        (-200)
#define FURI_HAL_POWER_VIRTUAL_CAPACITY_MAH (10000U)
#define BQ25896_CHARGE_LIMIT                1280

/* ---- Secondary OLED Display (SSD1306 0.96" I2C) ---- */
#define BOARD_HAS_OLED_SSD1306  1
#define BOARD_DISPLAY_SSD1306   1
#define BOARD_DISPLAY_I2C_PORT  I2C_NUM_0
#define BOARD_DISPLAY_I2C_ADDR  0x3C
#define BOARD_DISPLAY_I2C_FREQ_HZ 400000
#define BOARD_PIN_DISPLAY_SDA   47
#define BOARD_PIN_DISPLAY_SCL   42
#define BOARD_PIN_DISPLAY_RST   (-1)
