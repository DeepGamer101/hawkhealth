# Week 3 — Wokwi super-loop demo

A plain Arduino super-loop (no FreeRTOS) that tries to blink two LEDs at two different rates
in one loop() with delay() -- and can't. See `docs/week-04.md` for the lab.

**Run:** New Project -> STM32 -> ST Nucleo C031C6 -> paste `sketch.ino` -> add an LED on PB1
(anode -> PB1, cathode -> GND) -> Play. No library to add this week.

Watch the Serial Monitor: `A @ …` events are ~1000 ms apart, not 250 -- the two delays added
up and coupled the rates. That's the super-loop's fundamental limit.
