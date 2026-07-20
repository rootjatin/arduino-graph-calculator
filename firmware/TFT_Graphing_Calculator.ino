/*
  TFT Graphing Calculator - TouchSync v4 Arrows
  ------------------------------------------------
  Target: Arduino UNO R3 (ATmega328P) with an MCUFRIEND parallel TFT shield
          and 4-wire resistive touch panel.

  This file is intentionally small. Implementation is split into focused
  modules in the same sketch directory for easier maintenance.
*/

#include "GraphingCalculatorApp.h"

GraphingCalculatorApp app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}
