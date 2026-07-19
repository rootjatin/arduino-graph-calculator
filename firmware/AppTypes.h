#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>
#include "AppConfig.h"

struct CalibrationData {
  uint32_t magic;
  int16_t rawX[AppConfig::CAL_POINT_COUNT];
  int16_t rawY[AppConfig::CAL_POINT_COUNT];
  uint16_t checksum;
};

struct FloatPoint {
  float x;
  float y;
};

struct Button {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

enum ScreenMode {
  MODE_EDIT_MAIN,
  MODE_EDIT_FUNCTIONS,
  MODE_GRAPH
};

#endif
