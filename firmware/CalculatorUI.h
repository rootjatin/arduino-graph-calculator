#ifndef CALCULATOR_UI_H
#define CALCULATOR_UI_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

#include "AppTypes.h"
#include "GraphRenderer.h"

class CalculatorUI {
public:
  explicit CalculatorUI(MCUFRIEND_kbv &tft);

  void begin();
  void handleTouch(int16_t x, int16_t y);
  ScreenMode screenMode() const;

private:
  MCUFRIEND_kbv &tft_;
  GraphRenderer graph_;
  char expression_[AppConfig::MAX_EXPR_LEN + 1];
  uint8_t expressionLength_;
  ScreenMode screenMode_;

  void drawButton(const Button &button, const char *label, uint16_t fillColor);
  Button keypadButton(uint8_t row, uint8_t column) const;
  void drawExpressionBox();
  void drawEditScreen();

  bool appendText(const char *text);
  void setExpression(const char *text);
  void deleteLastCharacter();
  void showEditMessage(const char *message, uint16_t color);
  bool validateExpression() const;

  void handleMainKey(const char *label);
  void handleFunctionKey(uint8_t index, const char *label);
  void handleEditTouch(int16_t x, int16_t y);
  void handleGraphTouch(int16_t x, int16_t y);
  void drawGraphScreen();
};

#endif
