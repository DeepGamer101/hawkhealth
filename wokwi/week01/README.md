# Week 1 — Wokwi RTOS concept sketch

A two-task FreeRTOS demo for the browser simulator. See `docs/week-01.md` for the full lab.

**How to run:**
1. wokwi.com → **New Project → STM32 → ST Nucleo C031C6**.
2. Paste **`sketch.ino`**.
3. **Library Manager** tab → **+ Add** → **`STM32duino FreeRTOS`** (required — this provides
   `FreeRTOS.h` / `task.h`). The included `libraries.txt` lists it too.
4. In the diagram, add an **LED on PB1** (the on-board LED is PA5).
5. Press **▶ Play**.

You should see the on-board LED (PA5) blink fast, the PB1 LED blink slow, and interleaved
`A`/`B` lines in the Serial Monitor.

> Verified working in Wokwi. The Renode/hardware code in this repo is proven separately.
