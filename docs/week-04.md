# Week 4 Lab — RTOS Tasks & the Scheduler

> **Mentor's note.** The relief arrives. Last week the super-loop hit a wall; this week you meet the
> tool that gets past it — the RTOS scheduler — in the *real* HawkHealth code. You'll read how
> HawkHealth's jobs become independent **tasks**, and then you'll do the thing this course is really
> about: **inherit a defect and fix it.** The simulator is now **Renode** (from the emulator you set
> up in Week 2b), and you're working in the actual codebase from here on.

**By the end you will:** understand how HawkHealth's tasks and priorities are set up, and have
diagnosed and fixed your first inherited bug — turning a red CI check green.

---

## Part 1 — How HawkHealth becomes tasks (read the real code)

Open **`firmware/src/app/hh_app.c`** — this is where the whole system is wired up. Find the block of
`xTaskCreate(...)` calls near the bottom.

```c
xTaskCreate(HH_Sensor_Task,    "SENSOR", configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+2, NULL);
xTaskCreate(HH_Filter_Task,    "FILTER", configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+2, NULL);
xTaskCreate(HH_Alert_Task,     "ALERT",  configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+2, NULL);
xTaskCreate(HH_Telemetry_Task, "TELEM",  configMINIMAL_STACK_SIZE+64,  NULL, tskIDLE_PRIORITY+1, NULL);
xTaskCreate(HH_Command_Task,   "CMD",    configMINIMAL_STACK_SIZE+64,  NULL, tskIDLE_PRIORITY+3, NULL);
xTaskCreate(HH_Health_Task,    "HEALTH", configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+1, NULL);
```

Each line turns one job into an independent **task**. Read `xTaskCreate`'s arguments — they *are* the
lecture's task concepts, made concrete:

| Argument | What it is | Example above |
|---|---|---|
| function | the task's code (its `for(;;)` loop) | `HH_Sensor_Task` |
| name | a label (shows up in the stack-overflow message!) | `"SENSOR"` |
| **stack size** | words of RAM for this task's stack | `configMINIMAL_STACK_SIZE` |
| priority | who the scheduler runs first when several are ready | `tskIDLE_PRIORITY+2` |

**The key ideas to connect to lecture:**
- A task is either **running**, **ready**, or **blocked**. Most HawkHealth tasks spend most of their
  life *blocked* — waiting on a queue (`xQueueReceive`) or a delay (`vTaskDelay`). That's *good*: a
  blocked task uses no CPU, so others run.
- The **scheduler** runs the highest-priority **ready** task. When it blocks, the next one runs.
- This is exactly why HawkHealth can hold independent timings that the super-loop couldn't (Week 3):
  each task blocks on its *own* condition, and the scheduler interleaves them.

**Checkpoint 1.** In `hh_app.c`, point to a task's **stack size** and its **priority**, and explain
what "blocked" means for the SENSOR task (hint: it blocks when `rawQ` is full).

---

## Part 2 — Inherit a defect, and fix it (your first CI red → green)

This is the 20%-of-your-grade skill: you inherit real, broken code and make it work. Your instructor
has planted a defect in the task setup. Your job: reproduce it, diagnose it, fix it.

### 2a. Take the defect
Get this week's buggy version from your instructor (a `bug/week-04` branch to check out, or a
replacement `hh_app.c` to drop in). Commit it and push.

### 2b. Watch it fail
Open your repo's **Actions** tab. CI is now **red** — the **"Boots And Streams Telemetry"** test
fails. The system that ran fine last week no longer boots properly. *Something in the task setup
broke it.*

### 2c. See the symptom (Renode)
Download the firmware artifact and run it in Renode (`renode/hawkhealth.resc`, as in Week 2b). Watch
the `usart3` window:

```
[HawkHealth] system starting
[HH] t=1000 temp=36.8 spo2=98.0 hr=73 alert=NONE
[FATAL] stack overflow: TELEM
```

**Read that last line.** FreeRTOS's stack-overflow checker caught a task writing past its stack, and
the hook printed **which task**: `TELEM`. That's your lead — no guessing.

### 2d. Diagnose
Open `hh_app.c` and look at the `xTaskCreate` line for **TELEM**. Its stack size is too small for the
work it does (formatting numbers + the Cortex-M7 context). A too-small stack overflows at run time —
and FreeRTOS's `configCHECK_FOR_STACK_OVERFLOW` (set in `FreeRTOSConfig.h`) is what turned a silent
memory corruption into that clear message.

### 2e. Fix it
Give TELEM an adequate stack (the other "printing" task, CMD, is a good reference for how much).
Rebuild is automatic — commit on a branch, open a **pull request**, and watch CI.

### 2f. Confirm green
The **"Boots And Streams Telemetry"** test goes **green**. You've turned a red check green by
diagnosing and fixing inherited code — exactly the loop you'll repeat all term.

**Checkpoint 2.** Your PR fixes the stack overflow, CI is green, and you can explain in one sentence
*why* a too-small stack crashed TELEM and how the overflow hook named it for you.

---

## Exit criteria

- [ ] You can point to a task's stack size and priority in `hh_app.c` and explain "blocked."
- [ ] You reproduced the stack-overflow crash in Renode, fixed TELEM's stack, and CI is green.

## What to submit (weekly milestone + first inherited-defect fix)

1. A **screenshot** of the Renode `usart3` window showing the `[FATAL] stack overflow: TELEM` line
   (the bug reproduced).
2. The **link to your fix PR**, showing CI green.
3. **One or two sentences:** why did the too-small stack crash TELEM, and what does
   `configCHECK_FOR_STACK_OVERFLOW` do?
4. Your **AI Interaction Log** entry for Week 4.

## What's next

You've met tasks and fixed your first defect. **Week 5** goes deeper into the scheduler itself —
priorities, preemption, and starvation — where the choice of priority becomes the bug.
