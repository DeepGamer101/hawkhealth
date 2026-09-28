# Week 5 Lab — The Scheduler in Depth

> **Mentor's note.** Last week the RTOS caught your bug and told you exactly which task overflowed.
> This week you won't be so lucky. The defect you inherit is **silent** — the system prints its
> startup banner and then goes quiet, with no crash and no message. Fixing it takes one line; *finding*
> it takes reasoning about how the scheduler works. That reasoning is the real skill this week.

**By the end you will:** understand HawkHealth's task priorities and preemption, and have diagnosed
and fixed a **starvation** bug that gives you no error message to lean on.

---

## Part 1 — How the scheduler decides (read the priorities)

Open **`firmware/src/app/hh_app.c`** and find the `xTaskCreate(...)` block. The 5th argument of each
call is the task's **priority**. Read them off:

| Task | Priority | What it does each loop |
|---|---|---|
| COMMAND | `tskIDLE_PRIORITY + 3` (highest) | polls the UART, then **blocks** ~20 ms |
| SENSOR / FILTER / ALERT | `tskIDLE_PRIORITY + 2` | each **blocks** on its queue |
| TELEMETRY / HEALTH | `tskIDLE_PRIORITY + 1` (lowest) | **block** on a queue / a 500 ms delay |

**Two rules that explain everything this week:**
1. **The scheduler always runs the highest-priority task that is *ready*.** A lower-priority task
   runs only when every higher-priority task is **blocked**.
2. **Preemption:** the instant a higher-priority task becomes ready, it interrupts whatever is
   running. (This is the good part — it's how a critical task meets a deadline a super-loop couldn't.)

**The unwritten contract:** because the scheduler faithfully runs the highest-priority *ready* task,
**every task must block** — `vTaskDelay`, or waiting on a queue — so the others get a turn. Notice in
the table that *every* HawkHealth task blocks. A task that loops **without** blocking would never let
anything below it run.

> Note which task sits at the **top**: COMMAND, priority +3. Hold that thought.

**Checkpoint 1.** You can state HawkHealth's highest- and lowest-priority tasks, and explain why a
lower-priority task only runs when the higher ones are blocked.

---

## Part 2 — Diagnose and fix the silent hang (inherited defect)

### 2a. Take the defect
Get this week's buggy version from your instructor (a `bug/week-05` branch, or a replacement
`hh_command.c`). Commit it and push.

### 2b. Watch it fail — quietly
Open **Actions**. CI is **red**, but look how: the `system.robot` tests fail by **timing out**, not
by crashing. The system never produces the output the tests wait for.

### 2c. Reproduce it (Renode)
Download the firmware artifact and run it in Renode (`renode/hawkhealth.resc`). Watch the `usart3`
window:

```
[HawkHealth] system starting
```
…and then **nothing.** No telemetry, no `health monitor up`, no `[FATAL]` message. The banner prints
(that happens before the scheduler starts), and then the system goes silent. It looks hung.

> **This is the moment.** Last week the crash named the culprit. This week there's no message — so you
> reason it out.

### 2d. Reason it out (the actual skill)
Walk the logic:
- The banner printed, so `main()` ran and the scheduler started.
- But **nothing lower-priority is running** — no telemetry (TELEM is +1), no health line (HEALTH is +1),
  no alerts (ALERT is +2). They're all starved.
- Rule 1 says a lower task only runs when the higher ones are **blocked**. If *nothing* below the top
  runs, then **the highest-priority task must never be blocking** — it's hogging the CPU.
- The highest-priority task is **COMMAND** (+3). Open `firmware/src/command/hh_command.c` and read
  `HH_Command_Task`. **Does its loop ever block?** Compare it to every other task's loop.

You'll find COMMAND's loop is missing the yield that every task is supposed to have — so it spins
forever at the top priority and starves the whole system.

### 2e. Fix it
Restore the yield so COMMAND blocks each loop (the same `vTaskDelay` the other polling loops use),
letting lower-priority tasks run. Commit on a branch, open a **PR**, watch CI.

### 2f. Confirm green
Telemetry starts flowing again, `health monitor up` appears, and the `system.robot` tests go
**green**. You fixed a bug that never told you what it was.

**Checkpoint 2.** Your PR restores COMMAND's yield, CI is green, and you can explain — in one or two
sentences — how you concluded it was a starvation problem *without* any error message.

---

## Exit criteria

- [ ] You can read HawkHealth's task priorities and explain the "highest ready task runs" rule.
- [ ] You reproduced the silent hang, reasoned your way to the starving task, fixed it, and CI is green.

## What to submit (weekly milestone + inherited-defect fix)

1. A **screenshot** of the Renode `usart3` window showing the hang (`system starting`, then silence).
2. The **link to your fix PR**, showing CI green.
3. **One or two sentences:** how did you diagnose a starvation bug with no error message? (Name the
   reasoning, not just the fix.)
4. Your **AI Interaction Log** entry for Week 5.

## What's next

You debugged a fault with no message by reasoning about the scheduler. **Week 6** moves to **shared
data**: what happens when two tasks reach for the same thing at the same time — and the mutex that
keeps them honest.
