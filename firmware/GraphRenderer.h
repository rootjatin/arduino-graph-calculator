#ifndef GRAPH_RENDERER_H
#define GRAPH_RENDERER_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

#include "AppTypes.h"

class GraphRenderer {
public:
  explicit GraphRenderer(MCUFRIEND_kbv &tft);

  void draw(const char *expression, uint8_t expressionLength);
  void zoom(float factor);
  void pan(float xFraction, float yFraction);
  void resetView();

private:
  MCUFRIEND_kbv &tft_;
  float viewXMin_;
  float viewXMax_;
  float viewYMin_;
  float viewYMax_;

  float niceGridStep(float range) const;
  int16_t graphLeft() const;
  int16_t graphRight() const;
  int16_t graphTop() const;
  int16_t graphBottom() const;
  int16_t graphWidth() const;
  int16_t graphHeight() const;
  int16_t worldToPixelX(float x) const;
  int16_t worldToPixelY(float y) const;
  float pixelToWorldX(int16_t px) const;

  void drawGraphGrid();
  void plotExpression(const char *expression);
  Button toolbarButton(uint8_t index) const;
  void drawButton(const Button &button, const char *label, uint16_t fillColor);
};

#endif
