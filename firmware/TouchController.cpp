#include "TouchController.h"

#include <EEPROM.h>
#include <math.h>

#include "AppConfig.h"
#include "Utilities.h"

TouchController::TouchController(MCUFRIEND_kbv &tft, TouchScreen &touchScreen)
  : tft_(tft), touchScreen_(touchScreen) {
  calibration_.magic = 0;
  calibration_.checksum = 0;
}

void TouchController::ensureCalibration() {
  EEPROM.get(0, calibration_);
  bool forceCalibration = startupTouchRequestsCalibration();
  if (!calibrationIsValid() || forceCalibration) {
    runCalibrationWizard();
  }
}

uint16_t TouchController::calibrationChecksum(const CalibrationData &data) const {
  uint32_t sum = 0x5A5A;
  for (uint8_t i = 0; i < AppConfig::CAL_POINT_COUNT; ++i) {
    sum += static_cast<uint16_t>(data.rawX[i]);
    sum = (sum << 1) | (sum >> 31);
    sum += static_cast<uint16_t>(data.rawY[i]);
  }
  return static_cast<uint16_t>(sum ^ (sum >> 16));
}

bool TouchController::calibrationIsValid() const {
  if (calibration_.magic != AppConfig::CAL_MAGIC) return false;
  if (calibration_.checksum != calibrationChecksum(calibration_)) return false;

  int16_t minX = calibration_.rawX[0];
  int16_t maxX = calibration_.rawX[0];
  int16_t minY = calibration_.rawY[0];
  int16_t maxY = calibration_.rawY[0];

  for (uint8_t i = 1; i < AppConfig::CAL_POINT_COUNT; ++i) {
    if (calibration_.rawX[i] < minX) minX = calibration_.rawX[i];
    if (calibration_.rawX[i] > maxX) maxX = calibration_.rawX[i];
    if (calibration_.rawY[i] < minY) minY = calibration_.rawY[i];
    if (calibration_.rawY[i] > maxY) maxY = calibration_.rawY[i];
  }

  return (maxX - minX > 250) && (maxY - minY > 250);
}

void TouchController::restoreSharedPins() {
  pinMode(AppConfig::YP, OUTPUT);
  pinMode(AppConfig::XM, OUTPUT);
}

bool TouchController::readOneRawTouch(
  int16_t &rawX,
  int16_t &rawY,
  int16_t &pressure
) {
  TSPoint point = touchScreen_.getPoint();
  restoreSharedPins();

  pressure = point.z;
  if (pressure < AppConfig::MIN_PRESSURE ||
      pressure > AppConfig::MAX_PRESSURE) {
    return false;
  }

  rawX = point.x;
  rawY = point.y;
  return true;
}

bool TouchController::readFilteredRawTouch(
  int16_t &rawX,
  int16_t &rawY,
  int16_t &pressure
) {
  int16_t xs[AppConfig::TOUCH_SAMPLE_COUNT];
  int16_t ys[AppConfig::TOUCH_SAMPLE_COUNT];
  int16_t zs[AppConfig::TOUCH_SAMPLE_COUNT];
  uint8_t count = 0;

  for (uint8_t attempt = 0;
       attempt < AppConfig::TOUCH_SAMPLE_COUNT + 4 &&
       count < AppConfig::TOUCH_SAMPLE_COUNT;
       ++attempt) {
    int16_t x, y, z;
    if (readOneRawTouch(x, y, z)) {
      xs[count] = x;
      ys[count] = y;
      zs[count] = z;
      ++count;
    }
    delayMicroseconds(180);
  }

  if (count < AppConfig::TOUCH_REQUIRED_SAMPLES) return false;

  rawX = medianInt16(xs, count);
  rawY = medianInt16(ys, count);
  pressure = medianInt16(zs, count);
  return true;
}

int16_t TouchController::calibrationScreenX(uint8_t column) const {
  if (column == 0) return AppConfig::CAL_X_MARGIN;
  if (column == 1) return (tft_.width() - 1) / 2;
  return tft_.width() - 1 - AppConfig::CAL_X_MARGIN;
}

int16_t TouchController::calibrationScreenY(uint8_t row) const {
  if (row == 0) return AppConfig::CAL_Y_MARGIN;
  if (row == 1) return (tft_.height() - 1) / 2;
  return tft_.height() - 1 - AppConfig::CAL_Y_MARGIN;
}

void TouchController::drawCalibrationCross(
  int16_t x,
  int16_t y,
  uint16_t color
) {
  tft_.drawFastHLine(x - 8, y, 17, color);
  tft_.drawFastVLine(x, y - 8, 17, color);
  tft_.drawCircle(x, y, 4, color);
}

void TouchController::waitForRawRelease() {
  unsigned long releasedSince = 0;
  while (true) {
    int16_t x, y, z;
    bool down = readOneRawTouch(x, y, z);
    if (!down) {
      if (releasedSince == 0) releasedSince = millis();
      if (millis() - releasedSince > 120) return;
    } else {
      releasedSince = 0;
    }
    delay(8);
  }
}

bool TouchController::collectCalibrationPoint(int16_t &outX, int16_t &outY) {
  const uint8_t needed = 15;
  const uint8_t ignoredAtStart = 3;
  int16_t xs[needed];
  int16_t ys[needed];
  uint8_t stableSamples = 0;
  uint8_t ignoredSamples = 0;
  unsigned long start = millis();

  while (millis() - start < 15000UL) {
    int16_t x, y, z;
    if (readOneRawTouch(x, y, z)) {
      if (ignoredSamples < ignoredAtStart) {
        ++ignoredSamples;
      } else if (stableSamples < needed) {
        xs[stableSamples] = x;
        ys[stableSamples] = y;
        ++stableSamples;
      }

      if (stableSamples >= needed) {
        outX = medianInt16(xs, needed);
        outY = medianInt16(ys, needed);
        waitForRawRelease();
        return true;
      }
    } else {
      stableSamples = 0;
      ignoredSamples = 0;
    }
    delay(5);
  }

  return false;
}

void TouchController::runCalibrationWizard() {
  tft_.fillScreen(AppConfig::COLOR_BLACK);
  tft_.setTextColor(AppConfig::COLOR_WHITE, AppConfig::COLOR_BLACK);
  tft_.setTextSize(1);
  tft_.setCursor(20, 135);
  tft_.print("Hold each cross");
  tft_.setCursor(28, 153);
  tft_.print("Use plastic stylus");
  delay(1200);

  for (uint8_t row = 0; row < AppConfig::CAL_GRID; ++row) {
    for (uint8_t column = 0; column < AppConfig::CAL_GRID; ++column) {
      uint8_t index = row * AppConfig::CAL_GRID + column;
      int16_t sx = calibrationScreenX(column);
      int16_t sy = calibrationScreenY(row);

      tft_.fillScreen(AppConfig::COLOR_BLACK);
      drawCalibrationCross(sx, sy, AppConfig::COLOR_YELLOW);

      int16_t rx = 0;
      int16_t ry = 0;
      while (!collectCalibrationPoint(rx, ry)) {
        drawCalibrationCross(sx, sy, AppConfig::COLOR_RED);
        delay(180);
        drawCalibrationCross(sx, sy, AppConfig::COLOR_YELLOW);
      }

      calibration_.rawX[index] = rx;
      calibration_.rawY[index] = ry;
      drawCalibrationCross(sx, sy, AppConfig::COLOR_GREEN);
      delay(180);
      waitForRawRelease();
    }
  }

  calibration_.magic = AppConfig::CAL_MAGIC;
  calibration_.checksum = calibrationChecksum(calibration_);
  EEPROM.put(0, calibration_);

  tft_.fillScreen(AppConfig::COLOR_BLACK);
  tft_.setTextSize(2);
  tft_.setTextColor(AppConfig::COLOR_GREEN, AppConfig::COLOR_BLACK);
  tft_.setCursor(24, 130);
  tft_.print("Calibrated");
  delay(900);
}

bool TouchController::startupTouchRequestsCalibration() {
  tft_.fillScreen(AppConfig::COLOR_BLACK);
  tft_.setTextSize(1);
  tft_.setTextColor(AppConfig::COLOR_WHITE, AppConfig::COLOR_BLACK);
  tft_.setCursor(8, 120);
  tft_.print("Hold screen: calibrate");
  tft_.setCursor(8, 138);
  tft_.print("Starting...");

  unsigned long start = millis();
  unsigned long heldSince = 0;

  while (millis() - start < 1500UL) {
    int16_t rx, ry, z;
    bool down = readOneRawTouch(rx, ry, z);
    if (down) {
      if (heldSince == 0) heldSince = millis();
      if (millis() - heldSince > 600UL) {
        waitForRawRelease();
        return true;
      }
    } else {
      heldSince = 0;
    }
    delay(10);
  }

  return false;
}

float TouchController::outsidePenalty(float a, float b, float c) {
  float penalty = 0.0f;
  if (a < 0.0f) penalty -= a;
  if (b < 0.0f) penalty -= b;
  if (c < 0.0f) penalty -= c;
  if (a > 1.0f) penalty += a - 1.0f;
  if (b > 1.0f) penalty += b - 1.0f;
  if (c > 1.0f) penalty += c - 1.0f;
  return penalty;
}

bool TouchController::barycentric(
  const FloatPoint &p,
  const FloatPoint &a,
  const FloatPoint &b,
  const FloatPoint &c,
  float &wa,
  float &wb,
  float &wc
) {
  float denominator = (b.y - c.y) * (a.x - c.x) +
                      (c.x - b.x) * (a.y - c.y);
  if (fabs(denominator) < 0.0001f) return false;

  wa = ((b.y - c.y) * (p.x - c.x) +
        (c.x - b.x) * (p.y - c.y)) / denominator;

  wb = ((c.y - a.y) * (p.x - c.x) +
        (a.x - c.x) * (p.y - c.y)) / denominator;

  wc = 1.0f - wa - wb;
  return true;
}

void TouchController::calibrationRawPoint(
  uint8_t index,
  FloatPoint &point
) const {
  point.x = calibration_.rawX[index];
  point.y = calibration_.rawY[index];
}

void TouchController::calibrationDisplayPoint(
  uint8_t index,
  FloatPoint &point
) const {
  uint8_t row = index / AppConfig::CAL_GRID;
  uint8_t column = index % AppConfig::CAL_GRID;
  point.x = calibrationScreenX(column);
  point.y = calibrationScreenY(row);
}

void TouchController::testTriangleMapping(
  const FloatPoint &rawPoint,
  uint8_t ia,
  uint8_t ib,
  uint8_t ic,
  float &bestPenalty,
  float &bestX,
  float &bestY
) const {
  FloatPoint ra, rb, rc;
  FloatPoint sa, sb, sc;
  calibrationRawPoint(ia, ra);
  calibrationRawPoint(ib, rb);
  calibrationRawPoint(ic, rc);
  calibrationDisplayPoint(ia, sa);
  calibrationDisplayPoint(ib, sb);
  calibrationDisplayPoint(ic, sc);

  float wa, wb, wc;
  if (!barycentric(rawPoint, ra, rb, rc, wa, wb, wc)) return;

  float penalty = outsidePenalty(wa, wb, wc);
  if (penalty < bestPenalty) {
    bestPenalty = penalty;
    bestX = wa * sa.x + wb * sb.x + wc * sc.x;
    bestY = wa * sa.y + wb * sb.y + wc * sc.y;
  }
}

bool TouchController::mapRawToScreen(
  int16_t rawX,
  int16_t rawY,
  int16_t &screenX,
  int16_t &screenY
) const {
  FloatPoint rawPoint;
  rawPoint.x = rawX;
  rawPoint.y = rawY;

  float bestPenalty = 1000000.0f;
  float bestX = 0.0f;
  float bestY = 0.0f;

  for (uint8_t row = 0; row < 2; ++row) {
    for (uint8_t column = 0; column < 2; ++column) {
      uint8_t p00 = row * 3 + column;
      uint8_t p10 = p00 + 1;
      uint8_t p01 = p00 + 3;
      uint8_t p11 = p01 + 1;

      testTriangleMapping(
        rawPoint, p00, p10, p01, bestPenalty, bestX, bestY
      );
      testTriangleMapping(
        rawPoint, p10, p11, p01, bestPenalty, bestX, bestY
      );
    }
  }

  if (bestPenalty > 12.0f) return false;

  const float xStart = static_cast<float>(AppConfig::CAL_X_MARGIN);
  const float xEnd = static_cast<float>(
    tft_.width() - 1 - AppConfig::CAL_X_MARGIN
  );
  const float yStart = static_cast<float>(AppConfig::CAL_Y_MARGIN);
  const float yEnd = static_cast<float>(
    tft_.height() - 1 - AppConfig::CAL_Y_MARGIN
  );

  float fullX = (bestX - xStart) * static_cast<float>(tft_.width() - 1) /
                (xEnd - xStart);
  float fullY = (bestY - yStart) * static_cast<float>(tft_.height() - 1) /
                (yEnd - yStart);

  screenX = constrain(
    static_cast<int16_t>(fullX + 0.5f),
    static_cast<int16_t>(0),
    static_cast<int16_t>(tft_.width() - 1)
  );
  screenY = constrain(
    static_cast<int16_t>(fullY + 0.5f),
    static_cast<int16_t>(0),
    static_cast<int16_t>(tft_.height() - 1)
  );

  return true;
}

bool TouchController::getTouch(
  int16_t &screenX,
  int16_t &screenY,
  int16_t &pressure
) {
  int16_t rawX, rawY;
  if (!readFilteredRawTouch(rawX, rawY, pressure)) return false;
  return mapRawToScreen(rawX, rawY, screenX, screenY);
}
