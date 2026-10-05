# HawkHealth

A bedside patient-vitals monitor, and the codebase you'll live in this semester.

> **Dunwoody Hawks.** HawkHealth watches vitals the way a hawk watches — sharp senses,
> nothing missed. A monitor that stalls is a monitor that misses an alert.

## What's here

```
firmware/
  include/            public headers = the frozen interface surface
  src/
    sensor/           HH_Sensor_* : deterministic stub with injected faults
    platform/         hh_platform : USART3 + LED, register level (Renode AND board)
    app/              tasks, testbench, main, FreeRTOS hooks
    startup/          startup_stm32f767.c + stm32f767.ld
  third_party/        vendored FreeRTOS-Kernel (V11.3.1, Cortex-M7 port)
  FreeRTOSConfig.h    -> include/FreeRTOSConfig.h
  Makefile            arm-none-eabi-gcc, one command
platforms/            hawkhealth_f767.repl  (Renode STM32F767 model)
tests/                system.robot, integration.robot  (the CI gate)
.github/workflows/    ci.yml
docs/                 weekly lab guides
```

## One codebase, two build variants

```bash
make -C firmware            # hawkhealth.elf      -- the full monitor (SENSOR->FILTER->ALERT->TELEMETRY)
make -C firmware TEST=1     # hawkhealth_test.elf -- self-test build (integration testbench)
```

There is **no `PLATFORM_FVP` fork**. Because Renode models the real STM32F767, the same
`.elf` runs in the emulator and on the NUCLEO-F767ZI board.

## Run it in Renode (same as CI)

```bash
# one-time: install Renode 
renode-test tests/system.robot                  # boots, streams telemetry, injected faults raise alerts
renode-test tests/integration.robot             # TEST=1 build: 500 reads, PASS lines
```

Expected tail: `Tests finished successfully :)`

## The injected faults (what the testbench checks)

The sensor stub is deterministic — no `rand()` — so CI is reproducible:

| Call | Field | Value | Alert |
|------|-------|-------|-------|
| 300  | `temp_c`  | 38.5 | HIGH (>= 38.0) |
| 500  | `spo2_pct`| 88.0 | CRITICAL (< 90.0) |

## Verified

This foundation was built and run end-to-end before hand-off: both variants compile with
`arm-none-eabi-gcc` and pass their Renode Robot suites (FreeRTOS scheduler, SysTick tick,
USART3 telemetry, sensor stub, injected faults, PASS verdict).
Verification