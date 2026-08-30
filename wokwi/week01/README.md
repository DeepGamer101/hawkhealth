# Week 1 — Wokwi RTOS concept sketch

A two-task FreeRTOS demo for the browser simulator. See `docs/week-01.md` for the full lab.

**Run:** wokwi.com -> New Project -> STM32 -> ST Nucleo C031C6 -> paste `sketch.ino` ->
Library Manager -> + Add -> **STM32duino FreeRTOS** -> in the diagram add an LED on **PB1**
(anode -> PB1, cathode -> GND) -> Play.

You should see the on-board LED (PA5) blink fast, the PB1 LED blink slow, and interleaved
`A`/`B` lines in the Serial Monitor.

> Verified working: uses `#include <STM32FreeRTOS.h>` and calls `vTaskStartScheduler()` in setup().
