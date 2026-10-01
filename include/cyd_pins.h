#pragma once

// ESP32-S3 CYD 3.5" ST77922 hardware profile.
// Mirrors ESP32BusDash pin definitions for reuse.

namespace CydPins {
constexpr int CAN_RX = 45;
constexpr int CAN_TX = 46;

constexpr int SD_CLK = 5;
constexpr int SD_CMD = 4;
constexpr int SD_D0 = 6;

constexpr int LCD_CS = 10;
constexpr int LCD_BL = 41;
constexpr int LCD_SCLK = 12;
constexpr int LCD_D0 = 11;
constexpr int LCD_D1 = 13;
constexpr int LCD_D2 = 14;
constexpr int LCD_D3 = 9;

constexpr int TOUCH_SDA = 38;
constexpr int TOUCH_SCL = 39;
constexpr int TOUCH_RST = 48;
constexpr int TOUCH_INT = 47;

constexpr int STATUS_LED = 40;
constexpr int AUDIO_AMP_ENABLE = 1;
constexpr int MIC_PIN = 16;  // ADC1 channel for KWS input

constexpr int DISPLAY_WIDTH = 480;
constexpr int DISPLAY_HEIGHT = 320;
}
