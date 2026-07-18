# TFT Graphing Calculator for Arduino UNO

A modular Arduino UNO graphing calculator for an MCUFRIEND parallel TFT shield and a 4-wire resistive touchscreen.

This repository is a maintainable refactor of the original single-file sketch. The behavior is preserved while touch calibration, expression parsing, graph rendering, UI handling, and application lifecycle are separated into focused modules.

## Features

- 9-point touchscreen calibration stored in EEPROM
- Piecewise affine touch mapping for improved edge accuracy
- Median filtering and stable-press confirmation
- On-screen expression keypad
- Parser for `+`, `-`, `*`, `/`, `^`, parentheses, `x`, `pi`, and `e`
- Functions: `sin`, `cos`, `tan`, `sqrt`, `log`, `exp`, and `abs`
- Graph plotting, four-direction panning, zoom, and home/reset
- Compile-time guard for Arduino UNO / ATmega328P

## Repository layout

```text
.
├── firmware/
│   └── TFT_Graphing_Calculator/
│       ├── TFT_Graphing_Calculator.ino  # Arduino entry point
│       ├── GraphingCalculatorApp.*      # setup/loop and press tracking
│       ├── TouchController.*            # touch reading and calibration
│       ├── CalculatorUI.*               # keypad, expression editing, navigation
│       ├── GraphRenderer.*              # graph viewport and plotting
│       ├── ExpressionEngine.*           # expression parser/evaluator
│       ├── Utilities.*                  # shared small helpers
│       ├── AppConfig.h                   # pins, colors, limits, geometry
│       └── AppTypes.h                    # shared lightweight data types
├── docs/
│   └── ARCHITECTURE.md
├── platformio.ini
└── .gitignore
```

## Arduino IDE

1. Install these libraries with Library Manager:
   - **MCUFRIEND_kbv**
   - **Adafruit GFX Library**
   - **Adafruit TouchScreen**
2. Open:
   `firmware/TFT_Graphing_Calculator/TFT_Graphing_Calculator.ino`
3. Select **Tools → Board → Arduino AVR Boards → Arduino Uno**.
4. Select the correct serial port and upload.

All `.cpp` and `.h` files in the sketch directory are compiled automatically by the Arduino IDE.

## PlatformIO

From the repository root:

```bash
pio run
pio run --target upload
```

## Calibration

The calibration wizard runs automatically when no valid calibration exists in EEPROM.

To force recalibration, reset the Arduino and hold the touchscreen while the startup message is visible. Release when prompted, then hold the center of each cross using a plastic stylus.

## Hardware assumptions

- Arduino UNO R3 / ATmega328P
- MCUFRIEND-compatible parallel TFT shield
- Portrait orientation
- Common shield touch pins:
  - `XP = 6`
  - `XM = A2`
  - `YP = A1`
  - `YM = 7`

Adjust the pin constants in `AppConfig.h` only if your shield uses a different assignment.
