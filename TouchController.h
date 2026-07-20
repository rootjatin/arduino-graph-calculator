#ifndef TOUCH_CONTROLLER_H
#define TOUCH_CONTROLLER_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "AppTypes.h"

class TouchController {
public:
  TouchController(MCUFRIEND_kbv &tft, TouchScreen &touchScreen);

  void ensureCalibration();
  bool getTouch(int16_t &screenX, int16_t &screenY, int16_t &pressure);

private:
  MCUFRIEND_kbv &tft_;
  TouchScreen &touchScreen_;
  CalibrationData calibration_;

  uint16_t calibrationChecksum(const CalibrationData &data) const;
  bool calibrationIsValid() const;
  void restoreSharedPins();
  bool readOneRawTouch(int16_t &rawX, int16_t &rawY, int16_t &pressure);
  bool readFilteredRawTouch(int16_t &rawX, int16_t &rawY, int16_t &pressure);

  int16_t calibrationScreenX(uint8_t column) const;
  int16_t calibrationScreenY(uint8_t row) const;
  void drawCalibrationCross(int16_t x, int16_t y, uint16_t color);
  void waitForRawRelease();
  bool collectCalibrationPoint(int16_t &outX, int16_t &outY);
  void runCalibrationWizard();
  bool startupTouchRequestsCalibration();

  static float outsidePenalty(float a, float b, float c);
  static bool barycentric(
    const FloatPoint &p,
    const FloatPoint &a,
    const FloatPoint &b,
    const FloatPoint &c,
    float &wa,
    float &wb,
    float &wc
  );

  void calibrationRawPoint(uint8_t index, FloatPoint &point) const;
  void calibrationDisplayPoint(uint8_t index, FloatPoint &point) const;
  void testTriangleMapping(
    const FloatPoint &rawPoint,
    uint8_t ia,
    uint8_t ib,
    uint8_t ic,
    float &bestPenalty,
    float &bestX,
    float &bestY
  ) const;
  bool mapRawToScreen(
    int16_t rawX,
    int16_t rawY,
    int16_t &screenX,
    int16_t &screenY
  ) const;
};

#endif
