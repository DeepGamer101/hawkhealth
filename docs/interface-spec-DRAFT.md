# HawkHealth — Interface Specification (DRAFT for comment)

> **Status: DRAFT — circulated to all owners for review.** This is *not* frozen yet. Read it,
> especially your own subsystem and the interfaces you consume/produce, and propose changes now.
> After the comment period it **freezes** (Week 3): once frozen, changing a public signature needs a
> formal spec revision, because everyone codes against these contracts.

Each subsystem is **owned** by one student, who authors that subsystem's section and defends it at
the final oral review. The whole system runs in *every* student's repo, so these interfaces are the
shared contract that lets seven people build one system without stepping on each other.

## Owners (assign / claim before commenting)

| Subsystem | Header | Owner |
|---|---|---|
| S1 SENSOR | `hh_sensor.h` | __________ |
| S2 FILTER | `hh_filter.h` | __________ |
| S3 ALERT | `hh_alert.h` | __________ |
| S4 TELEMETRY | `hh_telemetry.h` | __________ |
| S5 COMMAND | `hh_command.h` | __________ |
| S6 CONFIG | `hh_config.h` | __________ |
| S7 HEALTH | `hh_health.h` | __________ |

## How to comment (due before the freeze)

1. **Read your own section** — do the functions and the "Produces/Consumes" match what your
   subsystem actually needs to do? Is the **queue type you produce** right (name, fields)?
2. **Read the neighbors you touch** — the queue you *consume* is someone else's *produce*. If your
   upstream/downstream doesn't give you what you need, say so now.
3. **Propose changes** [pick your class's channel: a comment on the spec PR / the shared doc /
   the Canvas discussion]. Be specific: which signature, what change, why.
4. **Deadline:** __________ . After that the spec freezes and changes require a revision request.

> Comment generously now — a five-minute change today is a cross-team headache after the freeze.

---

## The system at a glance

```
 [S1 SENSOR] --rawQ--> [S2 FILTER] --procQ--> [S3 ALERT] --telemetryQ--> [S4 TELEMETRY] --UART-->
                                                   ^
                                          reads    | thresholds
                                            [S6 CONFIG] <--writes-- [S5 COMMAND] <--UART RX
 [S7 HEALTH] watches every task's heartbeat, drives the status LED, feeds the watchdog
```

Data moves between tasks through **FreeRTOS queues**. **CONFIG** is shared state, reached only
through its mutex-guarded API. **HEALTH** watches everyone.

Shared types live in **`include/hh_types.h`**: `HH_SensorReading_t`, `HH_AlertLevel_t`
(`NONE`/`HIGH`/`CRITICAL`), `HH_TelemetryMsg_t` (a reading + its alert), and `HH_TaskId_t`.

---

## S1 — SENSOR   ·   `include/hh_sensor.h`   ·   Owner: ______

Acquires vital-sign readings on a fixed cadence and posts them to `rawQ`.

| Function | Contract |
|---|---|
| `HH_Sensor_Init()` | Prepare the sensor (or stub). Call once before the first read. |
| `HH_Sensor_Read(out)` | Fill `*out` with the next reading. Returns `HH_SENSOR_OK` on success; `HH_SENSOR_ERR_NOT_INIT` if read before init (sets `valid=false`). |
| `HH_Sensor_GetCallCount()` | How many reads since init (test support). |
| `HH_Sensor_Task(pv)` | Task: read on a cadence → post to `rawQ`. |

**Produces:** `HH_SensorReading_t` on `rawQ`.

---

## S2 — FILTER   ·   `include/hh_filter.h`   ·   Owner: ______

Validates and smooths raw readings before anything downstream trusts them.

| Function | Contract |
|---|---|
| `HH_Filter_Init()` | Reset any filter state. |
| `HH_Filter_Apply(raw)` | Pure function: return a validated/smoothed copy of `*raw`. |
| `HH_Filter_Task(pv)` | Task: `rawQ` → apply → `procQ`. |

**Consumes:** `rawQ`. **Produces:** `HH_SensorReading_t` on `procQ`.

---

## S3 — ALERT   ·   `include/hh_alert.h`   ·   Owner: ______

Compares a reading against the live thresholds (from CONFIG) and assigns a severity.

| Function | Contract |
|---|---|
| `HH_Alert_Init()` | Reset alert state. |
| `HH_Alert_Evaluate(r)` | Pure function: return `HH_ALERT_NONE/HIGH/CRITICAL` for `*r`, using CONFIG's current thresholds. |
| `HH_Alert_Task(pv)` | Task: `procQ` → evaluate → `telemetryQ` (reading + alert). |

**Consumes:** `procQ` (+ reads CONFIG). **Produces:** `HH_TelemetryMsg_t` on `telemetryQ`.

---

## S4 — TELEMETRY   ·   `include/hh_telemetry.h`   ·   Owner: ______

Formats readings + alerts and sends them out over the UART.

| Function | Contract |
|---|---|
| `HH_Telemetry_Init()` | Prepare output. |
| `HH_Telemetry_Send(m)` | Format one `HH_TelemetryMsg_t` and write it to the UART. |
| `HH_Telemetry_Task(pv)` | Task: `telemetryQ` → format → UART. |

**Consumes:** `telemetryQ`. **Produces:** bytes on USART3.

---

## S5 — COMMAND   ·   `include/hh_command.h`   ·   Owner: ______

Parses inbound UART commands (set a threshold, start/stop) and applies them via CONFIG.

| Function | Contract |
|---|---|
| `HH_Command_Init()` | Reset the parser. |
| `HH_Command_Feed(c)` | Accept one received byte (from the RX path/ISR). Non-blocking. |
| `HH_Command_Task(pv)` | Parse buffered input → CONFIG writes. |

**Consumes:** UART RX bytes. **Produces:** CONFIG writes.

---

## S6 — CONFIG   ·   `include/hh_config.h`   ·   Owner: ______

Owns the thresholds and the running/paused state. **All access is mutex-guarded** — this is the
shared-data lesson. ALERT reads it; COMMAND writes it.

| Function | Contract |
|---|---|
| `HH_Config_Init()` | Set defaults (`HH_DEFAULT_TEMP_HIGH_C`, `HH_DEFAULT_SPO2_CRIT_PCT`), create the mutex. |
| `HH_Config_GetThresholds(&t,&s)` | Read current thresholds (takes the mutex). |
| `HH_Config_SetTempHigh(t)` / `HH_Config_SetSpo2Critical(s)` | Update a threshold (takes the mutex). |
| `HH_Config_IsRunning()` / `HH_Config_SetRunning(b)` | Read/set run state (takes the mutex). |

**Consumes:** COMMAND writes. **Produces:** thresholds/state that ALERT reads.

---

## S7 — HEALTH   ·   `include/hh_health.h`   ·   Owner: ______

Watches every task's heartbeat, drives the status LED, and feeds the watchdog.

| Function | Contract |
|---|---|
| `HH_Health_Init()` | Reset the heartbeat table. |
| `HH_Health_Heartbeat(id)` | Each task calls this every loop to say "I'm alive." |
| `HH_Health_Task(pv)` | Periodically check all heartbeats; drive the status LED; (later) pet the watchdog. If a task goes silent, flag it. |

**Consumes:** heartbeats from every task. **Produces:** status LED + fault signal.

---

### Queues (created by the app at startup)

| Queue | From → To | Message type |
|---|---|---|
| `rawQ` | SENSOR → FILTER | `HH_SensorReading_t` |
| `procQ` | FILTER → ALERT | `HH_SensorReading_t` |
| `telemetryQ` | ALERT → TELEMETRY | `HH_TelemetryMsg_t` |
