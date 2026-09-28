# HawkHealth — Software Design

*An overall design of how the system works: what the pieces are, how data moves, how the parts run
concurrently, and why it's built this way. Read this with the interface spec (`interface-spec.md`),
which gives the exact function contracts.*

---

## 1. What HawkHealth is

HawkHealth is a **bedside patient-vitals monitor**. It continuously acquires vital signs, validates
them, checks them against configurable thresholds, streams the results out over a serial link, accepts
commands to reconfigure itself, and watches its own health. It runs **FreeRTOS on an STM32F767**
(Cortex-M7), and the *same* firmware runs in the Renode emulator and on the physical NUCLEO-F767ZI.

The guiding stakes: **a monitor that stalls is a monitor that misses an alert.** So the design favors
independent, non-blocking tasks and predictable real-time behavior over cleverness.

## 2. Design goals

- **Real-time & responsive** — a slow step in one job must never delay a critical one.
- **Modular** — seven subsystems, each independently owned, understood, and testable.
- **Portable** — application logic is decoupled from hardware behind a thin platform layer.
- **Testable & verifiable** — every change is built and exercised in CI against the emulator.
- **Observable & self-checking** — the system reports what it's doing and detects when a part goes quiet.

## 3. Architecture at a glance

Data flows one direction through a **pipeline** of tasks connected by queues. **CONFIG** is shared
state reached only through a guarded API; **COMMAND** feeds it from the outside; **HEALTH** watches
everyone.

```
                        rawQ            procQ            telemetryQ
   ┌────────┐  reading ┌────────┐ valid ┌────────┐ +alert ┌───────────┐   bytes
   │ SENSOR │ ───────▶ │ FILTER │ ─────▶│ ALERT  │ ──────▶│ TELEMETRY │ ────────▶ UART (USART3)
   └────────┘          └────────┘        └───┬────┘        └───────────┘
                                             │ reads thresholds
                                             ▼
                                        ┌─────────┐   writes    ┌─────────┐   RX bytes
                                        │ CONFIG  │ ◀────────── │ COMMAND │ ◀──────────  UART (USART3)
                                        │ (mutex) │             └─────────┘
                                        └─────────┘
   ┌────────┐
   │ HEALTH │  ◀── heartbeats from every task; drives the status LED; (watchdog on hardware)
   └────────┘
```

Two kinds of connection, chosen deliberately (see §6):
- **Queues** move data *forward* between pipeline stages (hand-off).
- **A mutex-guarded module (CONFIG)** holds data many tasks *share* (read by ALERT, written by COMMAND).

## 4. The seven subsystems

Each is owned by one student (interface-spec `Owner:` line). Six are FreeRTOS **tasks**; CONFIG is a
passive, mutex-guarded module with no task of its own.

| # | Subsystem | Responsibility | Task? |
|---|-----------|----------------|-------|
| S1 | **SENSOR** | Acquire a vitals reading on a fixed cadence; post to `rawQ`. | yes |
| S2 | **FILTER** | Validate / sanity-check a raw reading; forward to `procQ`. | yes |
| S3 | **ALERT** | Compare a reading to CONFIG's thresholds; assign NONE/HIGH/CRITICAL; post to `telemetryQ`. | yes |
| S4 | **TELEMETRY** | Format a reading + its alert and write it to the UART. | yes |
| S5 | **COMMAND** | Read inbound UART, parse commands, apply them to CONFIG. | yes |
| S6 | **CONFIG** | Hold thresholds + run-state; serve them through a mutex-guarded API. | no (passive) |
| S7 | **HEALTH** | Collect task heartbeats; drive the status LED; flag a silent task; feed the watchdog (hardware). | yes |

The **public headers** in `firmware/include/hh_*.h` are the frozen surface — everyone codes against
those, never against another subsystem's implementation.

## 5. Concurrency model

The scheduler always runs the **highest-priority READY task**; a task runs only when everything above
it is **blocked**. Every task spends most of its life blocked — on a queue or a delay — which is what
lets six tasks share one core cleanly.

| Task | Priority (`tskIDLE_PRIORITY +`) | Blocks on |
|------|-------------------------------|-----------|
| COMMAND | **+3** (highest) | a short delay between UART polls |
| SENSOR / FILTER / ALERT | +2 | queue full / empty (their in/out queue) |
| TELEMETRY / HEALTH | +1 (lowest) | queue empty / a periodic delay |

Rationale: COMMAND is responsive to operator input; the pipeline stages sit in the middle; telemetry
and health are important but not latency-critical. **The contract every task obeys: block regularly so
others run.** A task that spins without blocking starves everything beneath it.

## 6. Data flow & messages

Messages are plain structs (in `hh_types.h`), copied **by value** through the queues — so producer and
consumer must agree on the type and the queue must be created with the matching `sizeof`.

| Queue | From → To | Item type |
|-------|-----------|-----------|
| `rawQ` | SENSOR → FILTER | `HH_SensorReading_t` |
| `procQ` | FILTER → ALERT | `HH_SensorReading_t` (validated) |
| `telemetryQ` | ALERT → TELEMETRY | `HH_TelemetryMsg_t` (reading + `HH_AlertLevel_t`) |

`HH_SensorReading_t` = `{ timestamp_ms, temp_c, humidity_pct, spo2_pct, heart_rate_bpm, valid }`.
`HH_TelemetryMsg_t` = `{ reading, alert }` where `alert ∈ {NONE, HIGH, CRITICAL}`.

A queue **is** the synchronization: a consumer reading an empty queue simply blocks until data arrives
— no polling, no spinning.

## 7. Shared state & synchronization

**CONFIG** owns the live thresholds (`temp_high_c`, `spo2_critical_pct`) and the run flag. Because ALERT
reads them on every reading while COMMAND writes them on demand, all access goes through a **mutex**
(`xSemaphoreTake` / `Give`) so a read can never catch a half-finished write. Defaults:
`HH_DEFAULT_TEMP_HIGH_C = 38.0`, `HH_DEFAULT_SPO2_CRIT_PCT = 90.0`.

The rule of thumb the design follows: **if you can hand data off, use a queue (already thread-safe); if
tasks must share the same variable, use a mutex.** (Queues protect themselves via short critical
sections — you never add a mutex to a queue.)

## 8. Layering & portability (the seams)

The application logic is deliberately separated from the hardware by two **seams**:

- **Platform seam — `firmware/src/platform/hh_platform.c`.** All register-level I/O lives here: USART3
  (TX + RX) and the status LED, plus small print helpers (`hh_putc`, `hh_println`, `hh_print_u32`,
  `hh_print_float1`) and `hh_halt`. Swap this file and the tasks don't notice.
- **Sensor seam — `firmware/src/sensor/hh_sensor.c`.** Today a deterministic stub (with injected
  faults for teaching/testing); on hardware it becomes a real driver behind the same
  `HH_Sensor_Read()` contract.

This is why the *same task code* runs on a Cortex-M0+ in Wokwi, an emulated M7 in Renode, and real
silicon: **the RTOS/application layer is portable; only the I/O beneath the seams changes.** Tasks call
`HH_Sensor_Read()` and `hh_putc()`, never a sensor or a register.

## 9. Startup & lifecycle

1. **Reset** → the vector table (`startup_stm32f767.c`) loads the stack pointer and jumps to
   `Reset_Handler`, which enables the FPU, copies `.data`, zeroes `.bss`, and calls `main()`.
2. **`main()`** calls `hh_app_start()`.
3. **`hh_app_start()`** (in `firmware/src/app/hh_app.c`): init the platform, print the banner, init the
   subsystems (**CONFIG first**, since others read it), create the three queues, create the six tasks,
   then `vTaskStartScheduler()`.
4. **Running.** The scheduler takes over; tasks interleave. A `STOP`/`START` command toggles CONFIG's
   run flag, which gates the SENSOR task (pause/resume acquisition without tearing anything down).

Memory map (`stm32f767.ld`): flash @ `0x08000000` (2 MB), RAM @ `0x20000000` (512 KB). Kernel config
(`FreeRTOSConfig.h`): Cortex-M7, 4 priority bits, `heap_4`, stack-overflow checking on.

## 10. Error handling & reliability

- **HEALTH** collects a heartbeat from each task every loop; if a task goes silent past a timeout,
  HEALTH flags it. It also toggles the **status LED** as a visible "alive" signal, and on hardware pets
  the **watchdog** (a hung system then resets rather than lying quietly).
- **Stack-overflow hook** (`configCHECK_FOR_STACK_OVERFLOW`) catches a task overrunning its stack and
  prints the offending task's name before halting — turning silent corruption into a named fault.
- **Malloc-failed hook** catches heap exhaustion at task/queue creation.
- **Fatal path** — `hh_halt()` masks interrupts and stops, so a fault fails safe and loud rather than
  running on corrupted state.

## 11. Build, test & CI

- **One build** — a single `Makefile` + `arm-none-eabi-gcc`; the FreeRTOS kernel is vendored
  (Cortex-M7 port). Default build = the system; `make TEST=1` = the self-test build.
- **Emulation** — Renode runs the exact `.elf` with a modeled STM32F767 (`platforms/hawkhealth_f767.repl`).
- **CI** (`.github/workflows/ci.yml`) — on every push: build both variants, run the Renode suites
  (`tests/system.robot` end-to-end, `tests/integration.robot` sensor/testbench), and publish the
  firmware as a downloadable artifact. **Green CI is the definition of done.**

## 12. Extensibility

The system is built from a small set of reusable patterns (see the *Embedded Patterns* deck), so it
grows predictably:

- **Add an operation/stage** — define its message type, create a queue (`xQueueCreate(len, sizeof(type))`),
  add a task that consumes upstream and produces downstream, wire it in `hh_app_start()`.
- **Add shared config** — extend CONFIG's struct and its mutex-guarded getters/setters; never expose
  the raw variable.
- **Port to new hardware** — reimplement `hh_platform.c` (and the sensor driver) behind the existing
  contracts; the tasks are untouched.
- **Move input to interrupts** — on hardware, COMMAND's UART RX becomes an ISR that hands bytes to the
  task (the ISR→task pattern), replacing the poll — without changing COMMAND's parsing logic.

## 13. Key design decisions & rationale

| Decision | Why |
|----------|-----|
| Pipeline of tasks + queues | Decouples stages; each blocks on its own condition → independent timing, no shared-state hazards on the data path. |
| CONFIG as mutex-guarded module (not a queue) | Thresholds are *shared state* read repeatedly, not a one-way hand-off — a lock fits, a queue doesn't. |
| Thin platform/sensor seams | Portability across Wokwi → Renode → hardware with the same task code; also makes the stub↔real swap trivial and testable. |
| Deterministic sensor stub | Reproducible CI (no `rand()`), with injected faults that exercise the alert paths and seed teaching bugs. |
| Full system in every (student) repo | Each owner builds and integrates the *whole* system; integration is proven per-repo in CI, with no end-of-term merge. |
| Same `.elf` in emulator and on board | Renode gives fast, hardware-free CI; nothing about the build changes when moving to silicon. |

---

*This document describes the intended design. The authoritative details live in the code and in
`interface-spec.md`; if they ever disagree, the frozen headers win and this doc gets updated.*
