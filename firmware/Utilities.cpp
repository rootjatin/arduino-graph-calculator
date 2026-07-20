#include "Utilities.h"

int16_t medianInt16(int16_t *values, uint8_t count) {
  for (uint8_t i = 1; i < count; ++i) {
    int16_t key = values[i];
    int8_t j = static_cast<int8_t>(i) - 1;
    while (j >= 0 && values[j] > key) {
      values[j + 1] = values[j];
      --j;
    }
    values[j + 1] = key;
  }
  return values[count / 2];
}
