#include "GraphingCalculatorApp.h"

#include "Utilities.h"

GraphingCalculatorApp::GraphingCalculatorApp()
  : tft_(),
    touchScreen_(
      AppConfig::XP,
      AppConfig::YP,
      AppConfig::XM,
      AppConfig::YM,
      AppConfig::TOUCH_RESISTANCE_OHMS
    ),
    touchController_(tft_, touchScreen_),
    ui_(tft_),
    touchWasDown_(false),
    pressHandled_(false),
    pressSampleCount_(0),
    lastTouchReleaseMs_(0) {}

void GraphingCalculatorApp::setup() {
  uint16_t displayID = tft_.readID();
  if (displayID == 0xD3D3) displayID = 0x9486;

  tft_.begin(displayID);
  tft_.setRotation(AppConfig::SCREEN_ROTATION);
  tft_.fillScreen(AppConfig::COLOR_BLACK);

  touchController_.ensureCalibration();
  ui_.begin();
}

void GraphingCalculatorApp::loop() {
  int16_t x, y, pressure;
  bool touchDown = touchController_.getTouch(x, y, pressure);

  if (touchDown) {
    if (!touchWasDown_) {
      resetPressTracking();
    }

    if (!pressHandled_ && millis() - lastTouchReleaseMs_ > 70UL) {
      int16_t stableX, stableY;
      if (collectStablePressPoint(x, y, stableX, stableY)) {
        pressHandled_ = true;
        ui_.handleTouch(stableX, stableY);
      }
    }
  } else {
    if (touchWasDown_) {
      lastTouchReleaseMs_ = millis();
    }
    resetPressTracking();
  }

  touchWasDown_ = touchDown;
  delay(6);
}

void GraphingCalculatorApp::resetPressTracking() {
  pressSampleCount_ = 0;
  pressHandled_ = false;
}

bool GraphingCalculatorApp::collectStablePressPoint(
  int16_t x,
  int16_t y,
  int16_t &stableX,
  int16_t &stableY
) {
  if (pressSampleCount_ < AppConfig::PRESS_CONFIRM_SAMPLES) {
    pressSampleX_[pressSampleCount_] = x;
    pressSampleY_[pressSampleCount_] = y;
    ++pressSampleCount_;
  }

  if (pressSampleCount_ < AppConfig::PRESS_CONFIRM_SAMPLES) return false;

  int16_t minX = pressSampleX_[0];
  int16_t maxX = pressSampleX_[0];
  int16_t minY = pressSampleY_[0];
  int16_t maxY = pressSampleY_[0];

  for (uint8_t i = 1; i < AppConfig::PRESS_CONFIRM_SAMPLES; ++i) {
    if (pressSampleX_[i] < minX) minX = pressSampleX_[i];
    if (pressSampleX_[i] > maxX) maxX = pressSampleX_[i];
    if (pressSampleY_[i] < minY) minY = pressSampleY_[i];
    if (pressSampleY_[i] > maxY) maxY = pressSampleY_[i];
  }

  if ((maxX - minX) > AppConfig::PRESS_MAX_SPREAD_PIXELS ||
      (maxY - minY) > AppConfig::PRESS_MAX_SPREAD_PIXELS) {
    int16_t newestX =
      pressSampleX_[AppConfig::PRESS_CONFIRM_SAMPLES - 1];
    int16_t newestY =
      pressSampleY_[AppConfig::PRESS_CONFIRM_SAMPLES - 1];
    pressSampleX_[0] = newestX;
    pressSampleY_[0] = newestY;
    pressSampleCount_ = 1;
    return false;
  }

  stableX = medianInt16(
    pressSampleX_, AppConfig::PRESS_CONFIRM_SAMPLES
  );
  stableY = medianInt16(
    pressSampleY_, AppConfig::PRESS_CONFIRM_SAMPLES
  );
  return true;
}
