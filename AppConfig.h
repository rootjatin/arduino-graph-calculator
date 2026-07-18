#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>

// This project is specifically for Arduino UNO R3 / ATmega328P.
#if defined(ARDUINO) && !defined(__AVR_ATmega328P__)
  #error "Select Tools > Board > Arduino AVR Boards > Arduino Uno"
#endif

namespace AppConfig {

// Common UNO-style MCUFRIEND resistive-touch pin assignment.
constexpr int XP = 6;
constexpr int XM = A2;
constexpr int YP = A1;
constexpr int YM = 7;
constexpr int TOUCH_RESISTANCE_OHMS = 300;

constexpr uint8_t SCREEN_ROTATION = 0;

constexpr int16_t MIN_PRESSURE = 60;
constexpr int16_t MAX_PRESSURE = 4000;
constexpr uint8_t TOUCH_SAMPLE_COUNT = 3;
constexpr uint8_t TOUCH_REQUIRED_SAMPLES = 2;
constexpr uint8_t PRESS_CONFIRM_SAMPLES = 3;
constexpr int16_t PRESS_MAX_SPREAD_PIXELS = 20;

// RGB565 colors.
constexpr uint16_t COLOR_BLACK    = 0x0000;
constexpr uint16_t COLOR_WHITE    = 0xFFFF;
constexpr uint16_t COLOR_GRAY     = 0x8410;
constexpr uint16_t COLOR_DARKGRAY = 0x4208;
constexpr uint16_t COLOR_BLUE     = 0x001F;
constexpr uint16_t COLOR_CYAN     = 0x07FF;
constexpr uint16_t COLOR_GREEN    = 0x07E0;
constexpr uint16_t COLOR_RED      = 0xF800;
constexpr uint16_t COLOR_YELLOW   = 0xFFE0;
constexpr uint16_t COLOR_ORANGE   = 0xFD20;

constexpr uint8_t CAL_GRID = 3;
constexpr uint8_t CAL_POINT_COUNT = 9;
constexpr int16_t CAL_X_MARGIN = 16;
constexpr int16_t CAL_Y_MARGIN = 16;
constexpr uint32_t CAL_MAGIC = 0x47524334UL;  // "GRC4"

constexpr uint8_t MAX_EXPR_LEN = 47;
constexpr int16_t GRAPH_HEADER_H = 20;
constexpr int16_t GRAPH_TOOLBAR_H = 40;

}  // namespace AppConfig

#endif
