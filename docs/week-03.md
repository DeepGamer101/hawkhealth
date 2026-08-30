# Week 4 Lab — Understanding Super-Loops

> **Mentor's note.** Before you appreciate what an RTOS gives you, you have to feel life without
> one. This week you build the simplest possible "scheduler" — a super-loop — and run straight
> into its wall. It's a short lab with a big "aha," plus one milestone: **the interface spec
> freezes today.**

**By the end you will:** have watched a super-loop fail to keep two independent timings, understand
*why*, and have your interface-spec section finalized and frozen.

---

## Part 1 — The super-loop, and where it breaks (Wokwi)

A **super-loop** is `setup()` once, then everything else in one `loop()` that runs forever, doing
each job in sequence. No tasks, no scheduler — *you* are the scheduler. It's how nearly every
embedded system starts, and for simple jobs it's perfect.

We'll ask it to do something that sounds trivial: blink one LED every **250 ms** and another every
**750 ms** — at the same time.

1. In Wokwi: **New Project → STM32 → ST Nucleo C031C6** (same board as before).
2. Paste **[`wokwi/week04/sketch.ino`](../wokwi/week04/sketch.ino)**. *(No library to add this week —
   there's no FreeRTOS here. That's the point.)*
3. In the diagram, add an **LED on PB1** (anode → PB1, cathode → GND) — same as Weeks 1–2.
4. Press **▶ Play** and open the **Serial Monitor**.

**What you'll see — three failures at once:**
1. **The two blinkers couple.** Both LEDs blink at the *same* ~1 Hz rate, not 250 / 750 ms.
   Consecutive `A @ …` lines are ~**1000 ms** apart (`250 + 750`), because the delays run in
   sequence — A can't blink until B's `delay(750)` finishes.
2. **The CRITICAL job misses its deadline — by ~10×.** The `!! CRITICAL check` line prints its
   *actual* gap: it wants to run every **100 ms**, but the gap reads ~**1000 ms**. It only reaches
   the top of `loop()` once per pass. In a bedside monitor, that's a **missed alarm**.
3. **The timing JITTERS.** Every 4th pass a "slow sensor read" adds 400 ms, so the critical gap
   jumps to ~**1400 ms** — unpredictably. You can no longer say *when* the critical check will run.

**Why?** Every job waits its turn behind every other job's `delay()`. With just two blinkers you can
do the arithmetic (`250 + 750 = 1000`). But add a hard **100 ms deadline** and one **variable-time**
job, and the schedule is already impossible to predict — and real systems have *many* jobs. A
super-loop gives you no way to say "this critical check matters more than that slow read."

> **Cast your mind back to Week 1.** Two FreeRTOS *tasks* blinked at 250 ms and 750 ms perfectly and
> independently — because each task had its own `vTaskDelay`, and the scheduler ran them
> concurrently. That **decoupling** is the whole reason an RTOS exists. It's exactly what HawkHealth
> uses (the system you ran in Renode last week), and it's what Week 5 dives into.

**Checkpoint 1.** You can point to the `!! CRITICAL` line and say why its gap is ~1000 ms instead of
100 — and why adding jobs makes a super-loop's timing impossible to predict.

> *(Curious? A super-loop **can** juggle timings without `delay()` by checking `millis()` and doing
> work only when enough time has passed — but you have to hand-track every job's deadline yourself.
> That bookkeeping is exactly what the RTOS scheduler does for you. You don't need to build that
> this week — just know it's the trade.)*

---

## Part 2 — The interface spec FREEZES (milestone)

Your spec draft was due last week. **This week it locks.**

1. Finalize your subsystem's section in **`docs/interface-spec.md`** — signatures, what each function
   promises, what it consumes/produces. Make sure it matches your real header
   `firmware/include/hh_<name>.h` exactly.
2. Commit and open (or update) your PR; confirm **CI is green**.
3. **From now on, the spec is frozen.** Changing a public function signature requires a *spec
   revision* — a deliberate, announced change — because the whole system (and every teammate's
   mental model) is built against these contracts. This is exactly how a real team protects a shared
   interface.

**Checkpoint 2.** Your spec section is final, matches your header, and is merged (or in a green PR).

---

## Exit criteria

- [ ] The Wokwi super-loop runs; you've seen the two LEDs lock to the same rate and can explain why.
- [ ] Your interface-spec section is finalized (frozen) and in a green PR.

## What to submit (weekly milestone)

1. A **screenshot** of the Serial Monitor showing a `!! CRITICAL check` line whose **gap** is ~1000 ms
   (target 100) — the missed deadline.
2. **One or two sentences:** why does the CRITICAL job miss its 100 ms deadline, why does the timing
   jitter, and what decouples all of this?
3. The **link to your (final) interface-spec PR**.
4. Your **AI Interaction Log** entry for Week 4.

## What's next

You've felt the super-loop's wall. **Week 5** begins the RTOS: tasks and the scheduler — the tools
that make HawkHealth's independent, concurrent jobs possible. The simulator shifts from Wokwi to
**Renode**, and you'll start working in the real codebase.
