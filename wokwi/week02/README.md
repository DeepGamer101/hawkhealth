# Week 2 — Wokwi GPIO-input sketch

Blink + pushbutton on the Nucleo-C031C6. See `docs/week-02.md` for the full lab.

**Run:** New Project → STM32 → ST Nucleo C031C6 → paste `sketch.ino` → Library Manager →
add **STM32duino FreeRTOS** → in the diagram add a pushbutton (leg → PB2, diagonal leg → GND)
→ ▶ Play. The LED blinks; clicking the button prints `BUTTON pressed`.
