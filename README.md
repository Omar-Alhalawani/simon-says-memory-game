# Simon Says — Arduino Memory Game

**Arduino C++ • Embedded state management • LCD interface • EEPROM • Audio feedback**

A multi-mode, handheld-style **Simon Says memory game** built around an Arduino, four colored LED/button pairs, an I²C LCD, and a buzzer. The firmware generates progressively longer randomized sequences while tracking player lives, mode-specific behavior, and a persistent high score.

## Features implemented in the sketch

| Mode | Challenge |
| --- | --- |
| **Classic** | Repeat an increasingly long LED-and-tone sequence |
| **Speed** | Repeat the sequence with a **3-second time limit per input** |
| **Reverse** | Enter the displayed sequence backwards |
| **Stealth** | Memorize and reproduce a tone-only sequence |

Other implementation details:
- **Three lives** per game, with an incorrect input or timeout deducting one.
- **EEPROM high score** storage at address 0, surviving power cycles.
- **Debounced button polling** and distinct audio cues for modes, errors, and success.
- **Mute-at-start selection**, intro music, and LCD prompts.
- Random sequence generation seeded with an unused analog input.

## Hardware interface

| Component | Pins / configuration in source |
| --- | --- |
| Four LEDs | Digital pins 4, 5, 6, 7 |
| Four matching buttons | Digital pins 0, 1, 2, 3 (`INPUT_PULLUP`) |
| Piezo buzzer | Digital pin 10 |
| High-score reset button | Digital pin 8 |
| I²C LCD | Configured at address `0x3F` |
| Random seed | Analog input `A0` |

**Hardware caveat:** Arduino Uno pins **0 and 1 are shared with serial RX/TX**, so using them for buttons can conflict with uploads or serial communications. Confirm the intended wiring and address of the specific LCD module before use.

## Repository contents

```text
src/SimonGame_Final_JuneTwentyFive.ino  # Game logic and peripheral handling
docs/Simon_Manual.txt                  # Detailed game/user instructions
README.md                              # Overview and setup
```

## Build and setup

1. Install the **Arduino IDE**, the relevant Arduino board core, and a compatible `LiquidCrystal_I2C` library.
2. Open `src/SimonGame_Final_JuneTwentyFive.ino` and review the configured GPIOs and LCD address.
3. Supply the required `pitches.h` note-definition header (referenced by `#include "pitches.h"` but **not committed** in this repository).
4. Connect LEDs with appropriate current-limiting resistors, active-low buttons, buzzer, and display to match the sketch.
5. Compile, upload, and test each game mode on your own hardware.

The missing `pitches.h` means a fresh clone may **not compile without adding that dependency**.

## How to play

On startup, the system displays a welcome message and plays an intro. Select a mode using the four colored buttons: **yellow = Classic, green = Speed, red = Reverse, blue = Stealth**. Watch or listen to the pattern, then enter the corresponding sequence. Hold the dedicated high-score reset input during startup to clear the stored record.

## Known behavior and limitations

- The provided sketch blocks in the **game-over screen**; replay requires resetting/restarting the Arduino. The accompanying manual describes automatic replay, but the checked-in code does not implement that behavior.
- EEPROM supports a single high-score byte; this is not a networked leaderboard.
- The LCD constructor and geometry must be checked against the actual LCD/library version.
- No automated tests, wiring schematic, or media files are committed in this repository.

**Focus areas:** embedded C++, stateful interaction, GPIO, display integration, button debounce, nonvolatile memory, and user feedback.

See [the full user manual](docs/Simon_Manual.txt) for additional gameplay detail.
