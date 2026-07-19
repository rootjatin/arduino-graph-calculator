#include "CalculatorUI.h"

#include <string.h>

#include "AppConfig.h"
#include "ExpressionEngine.h"

namespace {

const char *MAIN_LABELS[24] = {
  "7", "8", "9", "/",
  "4", "5", "6", "*",
  "1", "2", "3", "-",
  "0", ".", "x", "+",
  "(", ")", "^", "DEL",
  "FUNC", "CLR", "PLOT", "pi"
};

const char *FUNCTION_LABELS[24] = {
  "sin(", "cos(", "tan(", "sqrt(",
  "log(", "exp(", "abs(", "pi",
  "x", "^", "(", ")",
  "+", "-", "*", "/",
  "DEL", "CLR", "MAIN", "PLOT",
  "x", "x^2", "sin(x)", "1/x"
};

}  // namespace

CalculatorUI::CalculatorUI(MCUFRIEND_kbv &tft)
  : tft_(tft),
    graph_(tft),
    expressionLength_(6),
    screenMode_(MODE_EDIT_MAIN) {
  strcpy(expression_, "sin(x)");
}

void CalculatorUI::begin() {
  drawEditScreen();
}

ScreenMode CalculatorUI::screenMode() const {
  return screenMode_;
}

void CalculatorUI::handleTouch(int16_t x, int16_t y) {
  if (screenMode_ == MODE_GRAPH) handleGraphTouch(x, y);
  else handleEditTouch(x, y);
}

void CalculatorUI::drawButton(
  const Button &button,
  const char *label,
  uint16_t fillColor
) {
  tft_.fillRect(
    button.x + 1,
    button.y + 1,
    button.w - 2,
    button.h - 2,
    fillColor
  );
  tft_.drawRect(
    button.x,
    button.y,
    button.w,
    button.h,
    AppConfig::COLOR_WHITE
  );

  uint8_t textSize = 1;
  size_t labelLength = strlen(label);
  if (labelLength == 1 && button.h >= 35) textSize = 2;
  else if (labelLength <= 3 && button.w >= 45 && button.h >= 35) textSize = 2;

  int16_t textWidth = static_cast<int16_t>(labelLength * 6 * textSize);
  int16_t textHeight = 8 * textSize;
  int16_t tx = button.x + (button.w - textWidth) / 2;
  int16_t ty = button.y + (button.h - textHeight) / 2;

  tft_.setTextSize(textSize);
  tft_.setTextColor(AppConfig::COLOR_WHITE, fillColor);
  tft_.setCursor(tx, ty);
  tft_.print(label);
}

Button CalculatorUI::keypadButton(uint8_t row, uint8_t column) const {
  const int16_t top = 62;
  const int16_t columns = 4;
  const int16_t rows = 6;
  int16_t width = tft_.width() / columns;
  int16_t height = (tft_.height() - top) / rows;

  Button button;
  button.x = column * width;
  button.y = top + row * height;
  button.w = (column == columns - 1) ? tft_.width() - button.x : width;
  button.h = (row == rows - 1) ? tft_.height() - button.y : height;
  return button;
}

void CalculatorUI::drawExpressionBox() {
  tft_.fillRect(0, 0, tft_.width(), 60, AppConfig::COLOR_BLACK);
  tft_.setTextColor(AppConfig::COLOR_CYAN, AppConfig::COLOR_BLACK);
  tft_.setTextSize(1);
  tft_.setCursor(4, 3);
  tft_.print(screenMode_ == MODE_EDIT_MAIN ? "EDIT" : "FUNCTIONS");
  tft_.print("  y=");

  tft_.drawRect(3, 16, tft_.width() - 6, 39, AppConfig::COLOR_WHITE);
  tft_.setTextColor(AppConfig::COLOR_YELLOW, AppConfig::COLOR_BLACK);
  tft_.setTextSize(1);
  tft_.setCursor(7, 23);

  const uint8_t visibleCharacters = 36;
  const char *start = expression_;
  if (expressionLength_ > visibleCharacters) {
    start = expression_ + expressionLength_ - visibleCharacters;
  }
  tft_.print(start);
}

void CalculatorUI::drawEditScreen() {
  tft_.fillScreen(AppConfig::COLOR_BLACK);
  drawExpressionBox();

  const char **labels =
    (screenMode_ == MODE_EDIT_MAIN) ? MAIN_LABELS : FUNCTION_LABELS;

  for (uint8_t row = 0; row < 6; ++row) {
    for (uint8_t column = 0; column < 4; ++column) {
      uint8_t index = row * 4 + column;
      Button button = keypadButton(row, column);
      uint16_t fill = AppConfig::COLOR_DARKGRAY;

      if (strcmp(labels[index], "PLOT") == 0) {
        fill = AppConfig::COLOR_GREEN;
      } else if (strcmp(labels[index], "CLR") == 0) {
        fill = AppConfig::COLOR_RED;
      } else if (strcmp(labels[index], "FUNC") == 0 ||
                 strcmp(labels[index], "MAIN") == 0) {
        fill = AppConfig::COLOR_BLUE;
      }

      drawButton(button, labels[index], fill);
    }
  }
}

bool CalculatorUI::appendText(const char *text) {
  uint8_t addedLength = strlen(text);
  if (expressionLength_ + addedLength > AppConfig::MAX_EXPR_LEN) return false;
  memcpy(expression_ + expressionLength_, text, addedLength + 1);
  expressionLength_ += addedLength;
  return true;
}

void CalculatorUI::setExpression(const char *text) {
  strncpy(expression_, text, AppConfig::MAX_EXPR_LEN);
  expression_[AppConfig::MAX_EXPR_LEN] = '\0';
  expressionLength_ = strlen(expression_);
}

void CalculatorUI::deleteLastCharacter() {
  if (expressionLength_ == 0) return;
  --expressionLength_;
  expression_[expressionLength_] = '\0';
}

void CalculatorUI::showEditMessage(
  const char *message,
  uint16_t color
) {
  tft_.fillRect(
    4, 44, tft_.width() - 8, 10, AppConfig::COLOR_BLACK
  );
  tft_.setTextSize(1);
  tft_.setTextColor(color, AppConfig::COLOR_BLACK);
  tft_.setCursor(7, 45);
  tft_.print(message);
}

bool CalculatorUI::validateExpression() const {
  float result;
  return ExpressionEngine::evaluate(expression_, 0.37f, result);
}

void CalculatorUI::drawGraphScreen() {
  graph_.draw(expression_, expressionLength_);
}

void CalculatorUI::handleMainKey(const char *label) {
  if (strcmp(label, "FUNC") == 0) {
    screenMode_ = MODE_EDIT_FUNCTIONS;
    drawEditScreen();
    return;
  }
  if (strcmp(label, "CLR") == 0) {
    setExpression("");
    drawExpressionBox();
    return;
  }
  if (strcmp(label, "DEL") == 0) {
    deleteLastCharacter();
    drawExpressionBox();
    return;
  }
  if (strcmp(label, "PLOT") == 0) {
    if (expressionLength_ == 0 || !validateExpression()) {
      showEditMessage("Invalid", AppConfig::COLOR_RED);
      return;
    }
    screenMode_ = MODE_GRAPH;
    drawGraphScreen();
    return;
  }

  if (!appendText(label)) {
    showEditMessage("Full", AppConfig::COLOR_RED);
  } else {
    drawExpressionBox();
  }
}

void CalculatorUI::handleFunctionKey(uint8_t index, const char *label) {
  if (strcmp(label, "MAIN") == 0) {
    screenMode_ = MODE_EDIT_MAIN;
    drawEditScreen();
    return;
  }
  if (strcmp(label, "CLR") == 0) {
    setExpression("");
    drawExpressionBox();
    return;
  }
  if (strcmp(label, "DEL") == 0) {
    deleteLastCharacter();
    drawExpressionBox();
    return;
  }
  if (strcmp(label, "PLOT") == 0) {
    if (expressionLength_ == 0 || !validateExpression()) {
      showEditMessage("Invalid", AppConfig::COLOR_RED);
      return;
    }
    screenMode_ = MODE_GRAPH;
    drawGraphScreen();
    return;
  }

  if (index == 20) setExpression("x");
  else if (index == 21) setExpression("x^2");
  else if (index == 22) setExpression("sin(x)");
  else if (index == 23) setExpression("1/x");
  else if (!appendText(label)) {
    showEditMessage("Full", AppConfig::COLOR_RED);
    return;
  }

  drawExpressionBox();
}

void CalculatorUI::handleEditTouch(int16_t x, int16_t y) {
  const int16_t keypadTop = 62;
  const uint8_t columns = 4;
  const uint8_t rows = 6;

  if (x < 0 || x >= tft_.width() ||
      y < keypadTop || y >= tft_.height()) {
    return;
  }

  uint8_t column = static_cast<uint8_t>(
    (static_cast<int32_t>(x) * columns) / tft_.width()
  );
  uint8_t row = static_cast<uint8_t>(
    (static_cast<int32_t>(y - keypadTop) * rows) /
    (tft_.height() - keypadTop)
  );

  if (column >= columns) column = columns - 1;
  if (row >= rows) row = rows - 1;

  uint8_t index = row * columns + column;

  if (screenMode_ == MODE_EDIT_MAIN) {
    handleMainKey(MAIN_LABELS[index]);
  } else {
    handleFunctionKey(index, FUNCTION_LABELS[index]);
  }
}

void CalculatorUI::handleGraphTouch(int16_t x, int16_t y) {
  const uint8_t toolbarCount = 8;
  if (x < 0 || x >= tft_.width() ||
      y < tft_.height() - AppConfig::GRAPH_TOOLBAR_H ||
      y >= tft_.height()) {
    return;
  }

  uint8_t index = static_cast<uint8_t>(
    (static_cast<int32_t>(x) * toolbarCount) / tft_.width()
  );
  if (index >= toolbarCount) index = toolbarCount - 1;

  switch (index) {
    case 0:
      screenMode_ = MODE_EDIT_MAIN;
      drawEditScreen();
      break;
    case 1:
      graph_.zoom(0.5f);
      drawGraphScreen();
      break;
    case 2:
      graph_.zoom(2.0f);
      drawGraphScreen();
      break;
    case 3:
      graph_.pan(-0.25f, 0.0f);
      drawGraphScreen();
      break;
    case 4:
      graph_.pan(0.25f, 0.0f);
      drawGraphScreen();
      break;
    case 5:
      graph_.pan(0.0f, 0.25f);
      drawGraphScreen();
      break;
    case 6:
      graph_.pan(0.0f, -0.25f);
      drawGraphScreen();
      break;
    case 7:
      graph_.resetView();
      drawGraphScreen();
      break;
  }
}
