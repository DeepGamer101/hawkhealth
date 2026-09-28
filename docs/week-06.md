# Week 6 Lab — Protecting Shared Data

> **Mentor's note.** The last two bugs showed themselves at boot — a crash, then a silent hang. This
> one is sneakier: the system runs **flawlessly**, and only freezes when you do a specific thing —
> change a config value. That "works until you poke it" behavior is the hallmark of a shared-data
> bug, and learning to hunt it is this week's skill. The fix is one line; finding it means following
> the trigger to the code.

**By the end you will:** understand how HawkHealth protects shared config with a mutex, and have
diagnosed and fixed a **deadlock** that only appears after a configuration change.

---

## Part 1 — How HawkHealth protects shared data

**CONFIG** holds the alert thresholds. Two tasks touch them: **ALERT reads** them on every reading
(to decide HIGH/CRITICAL), and **COMMAND writes** them whenever a UART command changes a threshold.
Same data, two tasks — a hazard. Without protection, a read could catch a write half-finished and see
an inconsistent snapshot.

The fix is a **mutex** (mutual exclusion): a single token you must **Take** before touching the data
and **Give** back when done, so a read and a write can never overlap.

Open **`firmware/src/config/hh_config.c`** and read the getters and setters. Each one follows the
same shape:

```c
void HH_Config_SetTempHigh(float t) {
    xSemaphoreTake(s_mtx, portMAX_DELAY);   // Take the token (enter)
    s_temp_high = t;                        // touch shared data
    xSemaphoreGive(s_mtx);                  // Give it back (leave)
}
```

**The one rule:** every `Take` needs a matching `Give`. And note `portMAX_DELAY` — a `Take` waits
**forever** for the token. So if a token is ever taken and *not* given back, the next task to `Take`
it blocks permanently. There's no timeout to rescue you.

**Checkpoint 1.** In `hh_config.c`, point to a `Take`/`Give` pair, and explain what `portMAX_DELAY`
means for a `Take` when the token is already held.

---

## Part 2 — Diagnose and fix the deadlock (inherited defect)

### 2a. Take the defect
Get this week's buggy version from your instructor (a `bug/week-06` branch, or a replacement
`hh_config.c`). Commit and push.

### 2b. Watch it fail — but only partly
Open **Actions**. Notice *which* test is red: the **command** test (it **times out**), while the boot
and injected-fault tests stay **green**. That's a huge clue — the system is fine *until something
sends a command.*

### 2c. Reproduce it (Renode)
Download the firmware artifact and run it in Renode (`renode/hawkhealth.resc`). Watch the `usart3`
window: telemetry streams normally — `[HH] t=… alert=…` lines, just like a healthy system.

Now **send a config command** and watch what happens. In the Renode Monitor, inject
`TEMP 30.0` + newline as received UART bytes:
```
sysbus.usart3 WriteChar 84
sysbus.usart3 WriteChar 69
sysbus.usart3 WriteChar 77
sysbus.usart3 WriteChar 80
sysbus.usart3 WriteChar 32
sysbus.usart3 WriteChar 51
sysbus.usart3 WriteChar 48
sysbus.usart3 WriteChar 46
sysbus.usart3 WriteChar 48
sysbus.usart3 WriteChar 10
```
You'll see the command's confirmation print — and then the telemetry **stops.** The system was
healthy right up until that command, and now it's frozen. No crash, no message.

### 2d. Diagnose (let the trigger point you)
The freeze happens **only after a config change** — so whatever a config change *touches* is where
the bug lives. Follow that path:
- A command that changes a threshold calls a **CONFIG setter**.
- Open `hh_config.c` and read that setter carefully. It **Takes** the mutex… **does it Give it
  back?** Compare it to the other functions in the file.
- With the token never returned, the **next** task to need it — ALERT, reading thresholds on the very
  next reading — `Take`s and blocks forever (`portMAX_DELAY`). ALERT stalls, the queues back up, and
  the whole pipeline freezes. That's your deadlock.

### 2e. Fix it
Restore the missing `xSemaphoreGive(s_mtx);` so the setter releases the token. Commit on a branch,
open a **PR**, watch CI.

### 2f. Confirm green
The command test passes — after a config change, telemetry keeps flowing. You fixed a bug that hid
behind perfectly normal operation.

**Checkpoint 2.** Your PR restores the `Give`, CI is green, and you can explain — in one or two
sentences — why the freeze happened *after a command* and not at boot.

---

## Exit criteria

- [ ] You can explain HawkHealth's mutex protection and the Take/Give rule (incl. what `portMAX_DELAY` implies).
- [ ] You reproduced the freeze-after-command, traced it to the unbalanced `Take`, fixed it, and CI is green.

## What to submit (weekly milestone + inherited-defect fix)

1. A **screenshot** of the Renode `usart3` window showing telemetry streaming and then stopping after
   the command (the deadlock reproduced).
2. The **link to your fix PR**, showing CI green.
3. **One or two sentences:** why did the system freeze *after* a config command rather than at boot?
4. Your **AI Interaction Log** entry for Week 6.

## What's next

You've now met all three RTOS failure modes — crash, starvation, deadlock. **Week 7** finishes the
RTOS core with **queues**, where the bug doesn't crash *or* freeze — the data just goes quietly wrong.
**Milestone 1** lands next week.
