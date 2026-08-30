# Week 1 Lab — Real-Time Systems & RTOS Concepts

> **Mentor's note.** Welcome to the team. You're not starting from a blank editor — you're
> joining a project already in motion. This week has two jobs, and neither is "write code":
> **(1) *see* concurrency** with your own eyes in Wokwi, and **(2) get oriented** in the
> HawkHealth codebase you'll grow all semester. Low stakes, high confidence. We build the
> habits before we lean on them.

**The idea that ties the whole course together — start noticing it today:** an RTOS is a
*portable layer*. The task you build in Wokwi this week is the same shape you'll run on the
STM32F767 in Renode (Week 5) and on real hardware (Week 9). Only the I/O underneath it
changes. Watch for that.

**By the end of this lab you will have:**
- two FreeRTOS tasks running (and pausable) in Wokwi, and
- the HawkHealth repo cloned, with your finger on two specific files.

---

## Part 0 — Tools & accounts (10 min)

You need three things. If you already have any from the CI walkthrough, skip ahead.

1. **GitHub account** — [github.com](https://github.com). You'll clone the repo and, later, open pull requests.
2. **Wokwi account** — [wokwi.com](https://wokwi.com) → *Sign up* (free for personal use). This is our browser simulator for RTOS concepts.
3. **Git** — either the `git` command line **or** [GitHub Desktop](https://desktop.github.com) (friendlier; it handles login for you).

**Checkpoint 0.** You can open both `github.com` and `wokwi.com`, signed in, on your Dunwoody laptop.

---

## Part 1 — See concurrency in Wokwi (the intuition)

**Goal:** watch two independent FreeRTOS tasks run at *different* rates on a real ST Nucleo,
and understand that neither one blocks the other. That "neither blocks the other" is the
whole point of an RTOS.

> **Why the C031C6 and not our F767?** Wokwi doesn't model the Nucleo-F767ZI — its STM32
> boards are smaller (the Nucleo-C031C6 is a Cortex-M0+). That's fine, and it's actually the
> lesson: the *same FreeRTOS task* runs on a tiny M0+ here and on our M7 later. Wokwi is the
> flight simulator; the real aircraft shows up in Renode in Week 5.

1. In Wokwi: **New Project → STM32 → ST Nucleo C031C6**. Wokwi creates the board for you.
2. Open the **`diagram.json`** tab and add a second LED: click **+** (parts), add an **LED**,
   and wire it to pin **PB1** (and its cathode to GND). The board's built-in LED is on **PA5**.
3. Open the **`sketch.ino`** tab and replace its contents with the file from the repo:
   **[`wokwi/week01/sketch.ino`](../wokwi/week01/sketch.ino)**.
4. **Add the FreeRTOS library.** Open the **Library Manager** tab (in the file list, next to
   `sketch.ino` / `diagram.json`) → click **+ Add** → search **`STM32duino FreeRTOS`** → add it.
   This is what makes `#include "FreeRTOS.h"` / `#include "task.h"` resolve. *(Skip this and the
   sketch won't compile — "FreeRTOS.h: No such file or directory.")*
5. Press the green **▶ Play** button.

**What you should see:**
- the **on-board LED (PA5)** toggles quickly — ~every 250 ms (Task A),
- the **PB1 LED** toggles slowly — ~every 750 ms (Task B),
- the **Serial Monitor** shows `A: tick` and `B: PING` lines *interleaved*, each on its own cadence.

That interleaving is the scheduler doing its job: two `for(;;)` loops that each think they own
the CPU, taking turns because each one **blocks** (`vTaskDelay`) instead of spinning.

> ⚠ **First-run note.** This sketch is matched to a *known-working* Wokwi C031C6 FreeRTOS
> project, but Wokwi's STM32 support moves around and I couldn't run it in my own sandbox — so
> the first play is a real verification. Two Wokwi quirks are already handled in the sketch:
> you include `FreeRTOS.h`/`task.h` directly, and you **don't** call `vTaskStartScheduler()`
> (Wokwi's Arduino core starts it for you after `setup()`). If the board or an include still
> errors, **stop and tell your instructor** — the fallback is the STM32 "blue pill" F103.
> Everything downstream (Renode, hardware) is already proven; this is the one rung we confirm live.

**Look at the task bodies and connect them to HawkHealth.** In the sketch, each task is:

```c
void TaskA(void *pv) {
    for (;;) {
        digitalWrite(LED_A, !digitalRead(LED_A));   // <-- I/O (changes per board)
        Serial.println("A: tick");                  // <-- I/O (changes per board)
        vTaskDelay(pdMS_TO_TICKS(250));             // <-- FreeRTOS (never changes)
    }
}
```

Now peek ahead at HawkHealth's blink task (`firmware/src/app/hawkhealth_hello.c`):

```c
void vTaskA_LedBlink(void *pv) {
    for (;;) {
        hh_led_toggle();                 // <-- I/O (the platform seam)
        vTaskDelay(pdMS_TO_TICKS(500));  // <-- FreeRTOS (identical call)
    }
}
```

Same `xTaskCreate` / `vTaskDelay` skeleton. **Only the I/O lines differ** — `digitalWrite`/`Serial`
here, `hh_led_toggle`/`hh_putc` there. That difference *is* the platform seam, and it's why the
same task survives the jump from an M0+ to an M7 to real silicon.

**Checkpoint 1.** Two LEDs blinking at different rates, and interleaved `A`/`B` lines in the
Serial Monitor.

---

## Part 2 — Pause it (a 60-second preview of debugging)

You don't need the debugger properly until Week 3, but feel it once now:

1. While the sim runs, click the **pause (⏸)** button.
2. Notice the sim *freezes mid-flight* — both tasks stop. That ability to stop time and look is
   what a debugger gives you, and it's how you'll diagnose real bugs later.
3. Press **▶** to resume.

**Checkpoint 2.** You paused and resumed the simulation.

---

## Part 3 — Meet the real codebase (HawkHealth)

**Goal:** get the repo onto your machine and learn its shape. No changes this week — reading is
the work.

1. **Clone the repo** (GitHub Desktop: *File → Clone repository → URL*, or `git clone <url>`).
   Ask your instructor for the HawkHealth repo URL.
2. **Walk the structure.** Open the folder and map it to the system diagram from lecture:

   | Folder | What lives there |
   |---|---|
   | `firmware/src/sensor/` | `HH_Sensor_Read()` — the sensor **stub** (synthetic data + injected faults) |
   | `firmware/src/platform/` | `hh_platform.c` — the **I/O seam** (USART3 + LED, register level) |
   | `firmware/src/app/` | the tasks, the integration testbench, `main` |
   | `firmware/include/` | the public headers — the **frozen interface surface** |
   | `tests/` | the Renode smoke + integration tests (the CI gate) |
   | `platforms/` | `hawkhealth_f767.repl` — the Renode model of our chip |

3. **Trace one reading, end to end.** Start in `firmware/src/sensor/sensor_hal.c` at
   `HH_Sensor_Read()` (where a reading is born), and follow where a task would send it out:
   `hh_println()` → `hh_putc()` in `firmware/src/platform/hh_platform.c` (where bytes hit
   USART3). You've just traced SENSOR → TELEMETRY.
4. **Find the two seams** — the two files that would change if you swapped the board or the
   sensor, while the tasks stayed put:
   - `firmware/src/platform/hh_platform.c` — the **I/O seam**
   - `firmware/src/sensor/sensor_hal.c` — the **stub ↔ real-sensor seam**

> *Optional, if you're curious:* the repo builds with one command if you have
> `arm-none-eabi-gcc` — `make -C firmware`. You don't need to today; we'll build it properly in
> Renode in Week 5. Reading is this week's job.

**Checkpoint 3.** You can point to `hh_platform.c` and say "this is the I/O seam," and to
`sensor_hal.c` and say "this is the sensor seam."

---

## Part 4 — Find your subsystem

You've been assigned one subsystem (S1–S7) to **own** — you'll write its interface-spec section
and defend it in the final oral review.

1. Find your subsystem's folder under `firmware/src/` and its public header in `firmware/include/`.
2. Read the header. Those function signatures are your subsystem's *contract* with the rest of
   the system — the thing everyone else codes against.

**Checkpoint 4.** You can name your subsystem's file(s) and its public functions.

---

## Exit criteria (what "done" looks like)

- [ ] Two FreeRTOS tasks running (and pausable) in Wokwi, blinking at different rates.
- [ ] HawkHealth repo cloned; you can point to `hh_platform.c` (I/O seam) and your subsystem's file.
- [ ] You started your **AI Interaction Log** for the week.

## What to submit (weekly milestone)

1. A **screenshot** of your Wokwi two-task sim running (both LEDs + the Serial Monitor).
2. One sentence: **which subsystem you own** and the name of its interface header.
3. Your **AI Interaction Log** entry for Week 1.

## What you are *not* doing yet

No code changes, no Renode, no hardware, no pull requests. This week is orientation and
intuition. Next week: the STM32 board and the professional developer toolchain.

---

*Stuck on any checkpoint? That's expected and it's what office hours are for — bring the exact
screen you're stuck on. "It's been a haul" is the normal feeling of real engineering; the habits
you build now make the hard weeks easy.*
