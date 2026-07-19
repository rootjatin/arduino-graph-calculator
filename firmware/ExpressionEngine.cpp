#include "ExpressionEngine.h"

#include <Arduino.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace {

class ExpressionParser {
public:
  ExpressionParser(const char *text, float xValue)
    : cursor_(text), x_(xValue), valid_(true) {}

  float evaluate() {
    float value = parseExpression();
    skipSpaces();
    if (*cursor_ != '\0') valid_ = false;
    if (isnan(value) || isinf(value)) valid_ = false;
    return value;
  }

  bool isValid() const { return valid_; }

private:
  const char *cursor_;
  float x_;
  bool valid_;

  void skipSpaces() {
    while (*cursor_ == ' ' || *cursor_ == '\t') ++cursor_;
  }

  bool consume(char expected) {
    skipSpaces();
    if (*cursor_ == expected) {
      ++cursor_;
      return true;
    }
    return false;
  }

  float parseExpression() {
    float value = parseTerm();
    while (valid_) {
      if (consume('+')) value += parseTerm();
      else if (consume('-')) value -= parseTerm();
      else break;
    }
    return value;
  }

  float parseTerm() {
    float value = parseUnary();
    while (valid_) {
      if (consume('*')) {
        value *= parseUnary();
      } else if (consume('/')) {
        float divisor = parseUnary();
        if (fabs(divisor) < 0.0000001f) {
          valid_ = false;
          return 0.0f;
        }
        value /= divisor;
      } else {
        break;
      }
    }
    return value;
  }

  float parseUnary() {
    skipSpaces();
    if (consume('+')) return parseUnary();
    if (consume('-')) return -parseUnary();
    return parsePower();
  }

  float parsePower() {
    float base = parsePrimary();
    if (consume('^')) {
      float exponent = parseUnary();
      base = pow(base, exponent);
      if (isnan(base) || isinf(base)) valid_ = false;
    }
    return base;
  }

  float parsePrimary() {
    skipSpaces();

    if (consume('(')) {
      float value = parseExpression();
      if (!consume(')')) valid_ = false;
      return value;
    }

    if (isdigit(*cursor_) || *cursor_ == '.') {
      char *endPointer;
      double value = strtod(cursor_, &endPointer);
      if (endPointer == cursor_) {
        valid_ = false;
        return 0.0f;
      }
      cursor_ = endPointer;
      return static_cast<float>(value);
    }

    if (isalpha(*cursor_)) {
      char identifier[8];
      uint8_t length = 0;
      while (isalpha(*cursor_) && length < sizeof(identifier) - 1) {
        identifier[length++] = static_cast<char>(tolower(*cursor_));
        ++cursor_;
      }
      identifier[length] = '\0';

      if (strcmp(identifier, "x") == 0) return x_;
      if (strcmp(identifier, "pi") == 0) return PI;
      if (strcmp(identifier, "e") == 0) return 2.718281828f;

      if (!consume('(')) {
        valid_ = false;
        return 0.0f;
      }

      float argument = parseExpression();
      if (!consume(')')) {
        valid_ = false;
        return 0.0f;
      }

      if (strcmp(identifier, "sin") == 0) return sin(argument);
      if (strcmp(identifier, "cos") == 0) return cos(argument);
      if (strcmp(identifier, "tan") == 0) return tan(argument);
      if (strcmp(identifier, "sqrt") == 0) {
        if (argument < 0.0f) valid_ = false;
        return sqrt(argument);
      }
      if (strcmp(identifier, "log") == 0) {
        if (argument <= 0.0f) valid_ = false;
        return log(argument);
      }
      if (strcmp(identifier, "exp") == 0) return exp(argument);
      if (strcmp(identifier, "abs") == 0) return fabs(argument);

      valid_ = false;
      return 0.0f;
    }

    valid_ = false;
    return 0.0f;
  }
};

}  // namespace

bool ExpressionEngine::evaluate(const char *text, float x, float &result) {
  ExpressionParser parser(text, x);
  result = parser.evaluate();
  return parser.isValid();
}
