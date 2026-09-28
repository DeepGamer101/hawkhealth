# Week 2a Lab (Paired) — Build a Pipeline: SENSOR → FILTER → ALERT

> **Mentor's note.** Last time you watched two tasks run. This time you build a real pipeline **with
> a partner** — and the hard part isn't the code, it's the *agreement*. I give you SENSOR. One of you
> writes **FILTER**, the other writes **ALERT**, and the two of you must **design the queue between
> them** so FILTER's output is exactly what ALERT needs. Get the contract right and it just works.
> Get it vague and ALERT reads garbage. That negotiation is the whole point.

**Pair up.** Decide who is **Student A (FILTER)** and **Student B (ALERT)**.

**By the end you will have:** a working SENSOR→FILTER→ALERT pipeline built from an interface contract
the two of you agreed on and wrote down — with each of you able to build and test your half *on your
own* before you connect them.

---

## What I give you: the SENSOR spec (fixed)

SENSOR is provided in the starter and **must not change**. It posts a `SensorSample_t` on `rawQ`
every 250 ms:

```c
typedef struct {
  uint32_t seq;        // sample number: 1, 2, 3, ...
  uint32_t t_ms;       // logical timestamp, ms
  float    temp_c;     // body temperature, C
  float    spo2_pct;   // blood-oxygen, %
  uint16_t hr_bpm;     // heart rate, bpm
} SensorSample_t;
```

**The stream is deliberately messy.** Mixed into the normal readings are:
- **Real events you SHOULD alert on:** a fever (`temp_c ≈ 38.6`) and a desat (`spo2_pct ≈ 89`).
- **Sensor glitches that are INVALID:** `temp_c = 250` and `spo2_pct = 0`. A good FILTER **rejects**
  them so ALERT never mistakes a glitch for an emergency.

That contrast is your success test: **real events must alert; glitches must not.**

---

## Step 0 — Run it and watch the mess (both of you)

Open the starter, make sure **STM32duino FreeRTOS** is added, and press ▶. The `RawPrinterTask` prints
the raw stream — clean readings, the occasional `250`/`0` glitch, the occasional real `38.6`/`89`.
*This is what FILTER has to clean up.*

## Step 1 — NEGOTIATE the contract (together, before any task code)

**ALERT states what it needs; FILTER states what it can provide; converge.** Fill in this worksheet
in a comment at the top of the sketch and both sign off. You have full freedom over the design.

```
========== FILTER → ALERT  INTERFACE AGREEMENT ==========
Queue name:            ________________   (e.g. filtQ)
Message type name:     ________________   (e.g. FilteredSample_t)

Fields  (name : type : units : meaning)
  ____________ : ______ : ______ : ______________________
  ____________ : ______ : ______ : ______________________
  ____________ : ______ : ______ : ______________________

FILTER guarantees (what every message it sends is true of):
  e.g. "only physiologically valid readings; temp in 30–45 C; spo2 in 50–100 %"
ALERT requires / assumes:
  e.g. "each message is already validated; I only apply thresholds"
How invalid samples are handled:  [ ] FILTER drops them  [ ] passed w/ valid-flag  [ ] other
Signed:  A (FILTER) __________     B (ALERT) __________
```

> Two classic traps: a **field mismatch** (ALERT expects something FILTER doesn't send) and a
> **semantic mismatch** (same fields, different meaning). Nail both now. A queue copies **by value**,
> so it must be created with the `sizeof` of the *exact* struct you both use.

**Checkpoint 1.** The filled-in agreement is in the sketch, and both partners approve it.

## Step 2 — Declare the contract in code (together)

In the **PAIR CONTRACT** region, turn your agreement into code:

```c
typedef struct { /* ... your agreed fields ... */ } FilteredSample_t;
QueueHandle_t filtQ;                 // FILTER -> ALERT
```
…and in `setup()`: `filtQ = xQueueCreate(8, sizeof(FilteredSample_t));`

> **This `FilteredSample_t` is the ONE definition you both build against.** Keep it identical on both
> sides — same fields, same order, same types. If it drifts, the halves won't connect.

## Step 3 — Split up and work in parallel (stub the other side)

You don't have to sit and take turns. Once the contract is fixed, **each of you fakes the other half**
so you can build and test alone — then delete the fakes when you connect. This is exactly how real
teams build against an agreed interface.

**Student B (ALERT) — fake the FILTER that feeds you.** Write a tiny producer that puts
`FilteredSample_t` values straight onto `filtQ`, forcing the cases you care about. Now you can test
ALERT with no dependency on A (and you get *better* coverage — you force a fever/desat on demand):

```c
// TEMP stub — stands in for A's FILTER. Delete at integration.
static void FakeFilterTask(void *pv) { (void)pv; FilteredSample_t f; uint32_t seq = 0;
  for (;;) { seq++;
    f.seq = seq; f.temp_c = 37.0f; f.spo2_pct = 98.0f;   // set the REST of your fields too
    if (seq % 3 == 0) f.temp_c   = 38.6f;   // force a fever -> expect HIGH
    if (seq % 4 == 0) f.spo2_pct = 89.0f;   // force a desat -> expect CRITICAL
    xQueueSend(filtQ, &f, portMAX_DELAY); vTaskDelay(pdMS_TO_TICKS(300)); } }
// setup(): create filtQ + FakeFilterTask + your AlertTask. (You don't need SENSOR/FILTER while solo.)
```

**Student A (FILTER) — fake the ALERT that consumes you.** You already have the real SENSOR, so just
put a printer on the end of `filtQ` to prove FILTER produces clean data:

```c
// TEMP stub — stands in for B's ALERT. Delete at integration.
static void FakeAlertTask(void *pv) { (void)pv; FilteredSample_t f;
  for (;;) if (xQueueReceive(filtQ, &f, portMAX_DELAY) == pdTRUE) {
    Serial.print("FILTERED seq="); Serial.print(f.seq);
    Serial.print(" temp=");        Serial.print(f.temp_c);
    Serial.print(" spo2=");        Serial.println(f.spo2_pct); } }   // no 250/0 should ever appear
// setup(): create SENSOR + your FilterTask + FakeAlertTask. (Delete RawPrinterTask.)
```

Both stubs are already in the starter (commented out) — uncomment the one you need.

**The rule that makes this work:** your `FilteredSample_t` must be **byte-for-byte identical** in both
projects. Two ways to guarantee that:
- **Simplest:** agree the struct in Step 2, then each of you pastes the **exact same** struct into your
  own project — and nobody edits it alone.
- **Cleaner (the real-world way):** put the struct in its own file `contract.h` and both `#include`
  it. Changing it is a renegotiation. (Wokwi lets you add tabs/files — this previews how the real
  HawkHealth freezes its interfaces in `firmware/include/`.)

**Checkpoint 3.** Each partner can run and test their half alone, against the stub.

## Step 4 — Student A: finish FILTER (against the fake ALERT)

Receive from `rawQ`, **reject the glitches**, build a `FilteredSample_t`, `xQueueSend` on `filtQ`.
Watch `FakeAlertTask`'s output: only clean readings should appear (no `250`/`0`). Delete
`RawPrinterTask`.

## Step 5 — Student B: finish ALERT (against the fake FILTER)

Receive from `filtQ`, apply thresholds, print the decision. Suggested (change if you agreed
otherwise): `temp >= 38.0 -> HIGH`, `spo2 < 90 -> CRITICAL`, else `OK`. Watch `FakeFilterTask`'s forced
fever/desat produce `HIGH`/`CRITICAL`.

## Step 6 — Integrate & verify

Bring the two real halves together: **delete BOTH fakes** (and `RawPrinterTask`), wire
SENSOR → FILTER → ALERT on the shared `filtQ`, and run. You pass when:
- **Real events alert:** `HIGH` on the `38.6` fever, `CRITICAL` on the `89` desat.
- **Glitches do NOT alert:** the `250`/`0` samples produce **no** alert.

If it connects and passes, your contract held. **If it doesn't connect, the mismatch is a contract
disagreement** — fix the *contract*, not just the code. That's the lesson.

---

## Working together — two ways

- **(a) Co-located pairing** — one screen, one **driver** (types), one **navigator** (reads the spec,
  catches mistakes); swap at Step 4/5. Simplest.
- **(b) Parallel with stubs (Step 3)** — each in your own Wokwi project, build against the fake,
  integrate at the end. This is the professional workflow.

Either way, the design decisions (Steps 1–2) are made **together**, and one shared Wokwi project holds
the final integrated pipeline (share its link for submission).

## Exit criteria
- [ ] Filled-in Interface Agreement in the sketch, approved by both.
- [ ] Integrated pipeline runs: real events alert (`HIGH`/`CRITICAL`); glitches produce no alert.

## What to submit (per pair)
1. Both partners' names, and who owned FILTER vs ALERT.
2. The **shared Wokwi project link** (integrated).
3. A **screenshot** of the Serial Monitor showing a real alert firing and a glitch (`250`/`0`) passing
   without an alert.
4. Your **Interface Agreement** (paste it) — one to two sentences on the trickiest thing you agreed on.
5. Each partner's **AI Interaction Log** entry.

## What's next
You just negotiated a queue contract with one partner and built against it in parallel. In the full
course, seven of you agree on the whole system's interfaces at once — which is why next you'll help
review the **HawkHealth interface spec**. Same skill, bigger table.
