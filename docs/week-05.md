# Week 5 Lab — Your Module on Renode, and the Gate That Protects the Baseline

> **Mentor's note.** You've done the design work: your subsystem's contract is frozen, you've
> reasoned about priorities, and you've felt tasks-and-queues move data on Wokwi. Now you stop
> sketching and start *building* — on the same emulated STM32F767 that CI grades, running on your
> own machine. This week is less about a new RTOS concept and more about the **loop you'll live in
> for the rest of the course**: write code → **prove it on Renode locally** → *only then* commit.
> Renode is your wind tunnel. You don't push a wing to the flight line because it looks right; you
> put it in the tunnel first. "It compiles" is not "it works," and "it works on my machine" is not
> "it's safe for the team's baseline." By the end of this lab, the difference between those is a
> command you can run, and a gate the repo enforces for you.

**By the end you will:**

- run the real HawkHealth firmware on **Renode, on your own machine**, two ways — *watch* it stream
  telemetry, and *test* it exactly the way CI does;
- read the **SENSOR → FILTER → ALERT** slice, trace one reading through it, and make a change you
  prove on Renode *before* it leaves your laptop;
- work the **branch → local-green → PR → CI-green → merge** loop, with CI standing as the enforced
  gate on the shared baseline;
- pull a **classmate's finished subsystem** into your repo through the frozen contract, and prove
  the integration locally.

> **Prereqs.** You already have your own private `hawkhealth-<name>` repo cloned (see
> `docs/student-onboarding.md`) and Renode installed. This lab assumes the class-standard build
> **`renode_1.16.1+20260828git00139efee-portable`** — the specific nightly we validated against the
> NUCLEO-F767ZI, and the one our CI runs. It is *not* the stable `1.16.1` from the GitHub releases
> page; it's a dated nightly from `builds.renode.io`. Everyone on the team runs the **same** build,
> because "green on Renode" only means something if it's the same Renode. Check yours in Part 0.

---

## Part 0 — Confirm your toolchain (two commands)

You installed these already; this is just a preflight so nothing surprises you mid-lab. Open a
terminal **in your clone's root** (the folder with `firmware/`, `tests/`, `platforms/`).

```bash
arm-none-eabi-gcc --version      # the compiler (CubeIDE ships one; or a standalone Arm GNU toolchain)
renode --version                 # the emulator
```

Two things to check:

- **The compiler prints a version.** If "command not found," add CubeIDE's bundled
  `arm-none-eabi-gcc` to your PATH (or install the standalone Arm GNU toolchain). You need it to
  build the firmware locally.
- **Renode reports the build `1.16.1+20260828git00139efee`.** The version line includes that
  `+<date>git<hash>` tag — that's how you tell the nightly apart from the stable `1.16.1` release
  (they're different binaries, and only the nightly is board-validated). If yours differs, you and CI
  are running *different* emulators, which defeats the whole point of a local gate. Install the
  class-standard nightly from `builds.renode.io` (see Part 5's version note for the exact file). If
  the class later moves to a newer nightly for the board, CI moves with you — one line, same note.
  The rule never changes: **local Renode and CI Renode are the same build.**

**Checkpoint 0.** Both commands report a version, and your Renode build string matches the one this
repo's CI installs.

---

## Part 1 — Build the firmware locally

One `Makefile`, one compiler, two variants. From your clone root:

```bash
make -C firmware            # -> firmware/hawkhealth.elf       the system (the real pipeline)
make -C firmware TEST=1     # -> firmware/hawkhealth_test.elf  the self-test build (the testbench)
```

- **`hawkhealth.elf`** is the actual system: all seven subsystems, the queues, the scheduler — the
  thing you watch stream telemetry.
- **`hawkhealth_test.elf`** is the same code compiled with `-DHH_BUILD_TEST`, which swaps in an
  in-firmware testbench that drives a fixed number of readings and prints machine-checkable
  `PASS`/`FAIL` lines. CI builds *both*; so do you.

There is no separate "simulator fork." Because Renode models a real STM32F767, the **exact same
`.elf`** runs in the emulator today and on the NUCLEO-F767ZI board later. Nothing about the build
changes when you move to silicon — that's the payoff of the platform seam
(`firmware/src/platform/hh_platform.c`).

**Checkpoint 1.** Both builds succeed and you can see `firmware/hawkhealth.elf` and
`firmware/hawkhealth_test.elf` on disk.

---

## Part 2 — Run it on Renode, two ways

There are two reasons to run firmware in Renode, and you'll use both all semester. **Watching** is
for understanding and debugging — you see the system behave. **Testing** is for proving — a script
checks the behavior and returns pass/fail. Watching is how you develop; testing is what gates your
commits.

### 2a. Watch it — see the pipeline alive (the GUI)

The repo ships a Renode script, `renode/hawkhealth.resc`, that loads the board model and your
firmware and opens the UART so you can read the telemetry. Open it and point the two `@` paths at
**your** files — forward slashes, no quotes, on every OS:

```
# renode/hawkhealth.resc  (edit these two lines)
machine LoadPlatformDescription @/absolute/path/to/your-clone/platforms/hawkhealth_f767.repl
sysbus LoadELF               @/absolute/path/to/your-clone/firmware/hawkhealth.elf
```

*(Windows example: `@C:/Users/you/CENG4200/hawkhealth-<name>/firmware/hawkhealth.elf`. Yes, forward
slashes even on Windows — Renode wants them.)*

Then launch Renode, load the script, and start the machine:

```
(monitor) i @/absolute/path/to/your-clone/renode/hawkhealth.resc
(monitor) start
```

A **usart3** analyzer window opens and telemetry starts scrolling. This is your subsystem's world:
SENSOR is producing readings, FILTER is validating them, ALERT is scoring them, TELEMETRY is
printing them. Watch for the two **injected faults** the sensor stub plants on a fixed schedule:

```
[HH] t=300000 temp=38.5 ... alert=HIGH        <- call 300: temp crosses 38.0
[HH] t=500000 spo2=88.0  ... alert=CRITICAL    <- call 500: SpO2 drops below 90.0
```

Seeing `HIGH` at the 38.5 reading and `CRITICAL` at the 88.0 reading means the whole
SENSOR→FILTER→ALERT→TELEMETRY path is intact and the live thresholds are being read correctly. Type
`quit` (or close the window) when you've seen enough.

> **Tip.** You can also load the ELF straight from your clone with a path *relative to where you
> launched Renode*: start Renode from the clone root and the shipped absolute paths become
> unnecessary. Absolute paths always work; relative ones are just less to edit.

### 2b. Test it — the exact command CI runs (this is your gate)

Watching is subjective. The gate is not. `renode-test` runs a Robot Framework suite that boots the
firmware, watches the UART, and asserts on what it sees. From your clone root:

```bash
renode-test tests/system.robot          # end-to-end: boot, injected faults, command behavior
renode-test tests/integration.robot     # the TEST=1 testbench: 500 reads, PASS lines
```

*(If your Renode is a portable/unzipped install rather than on your PATH, call it by its path —
e.g. `./renode/renode-test tests/system.robot`. On Windows, run it from the Renode-provided
terminal so `renode-test` resolves.)*

The tail you're looking for:

```
Tests finished successfully :)
```

That line — on **both** suites — is the definition of "safe to commit." `system.robot` checks three
things, and it's worth knowing what each one guards:

| Test | What it proves | Which subsystem it leans on |
|------|----------------|------------------------------|
| `Boots And Streams Telemetry` | the system comes up and produces output | the whole pipeline |
| `Injected Faults Raise Correct Alerts` | 38.5 → HIGH, 88.0 → CRITICAL | SENSOR (injects) → FILTER (passes) → ALERT (scores) |
| `Command Changes Alert Behavior` | a UART `TEMP 30.0` retunes the threshold live | COMMAND → CONFIG → ALERT |

`integration.robot` runs the `hawkhealth_test.elf` you built with `TEST=1` and looks for
`T3 HIGH temp @ call300`, `T4 CRIT spo2 @ call500`, and finally `HAWKHEALTH_TEST_PASS`.

**Checkpoint 2.** You've *watched* telemetry stream and seen the two injected alerts, and you've run
both robot suites to `Tests finished successfully :)`.

---

## Part 3 — The worked slice: SENSOR → FILTER → ALERT

This is the stretch of pipeline where most of you own a stage, so read it as the owner *and* as the
neighbor. Three tasks, two queues between them, one reading flowing left to right.

**SENSOR** (`firmware/src/sensor/hh_sensor.c`) produces onto `rawQ`:

```c
void HH_Sensor_Task(void *pv){
    HH_SensorReading_t r;
    for(;;){
        if(HH_Config_IsRunning() && HH_Sensor_Read(&r) == HH_SENSOR_OK){
            (void)xQueueSend(rawQ, &r, portMAX_DELAY);   // hand the reading forward
        }
        HH_Health_Heartbeat(HH_TASK_SENSOR);
        vTaskDelay(pdMS_TO_TICKS(10));                   // sim cadence; logical time is in timestamp_ms
    }
}
```

The stub is **deterministic** — no `rand()`, so every run is identical and CI is reproducible. It
plants the two faults you watched: `temp_c = 38.5` on call 300, `spo2_pct = 88.0` on call 500.

**FILTER** (`firmware/src/filter/hh_filter.c`) consumes `rawQ`, validates, produces `procQ`:

```c
HH_SensorReading_t HH_Filter_Apply(const HH_SensorReading_t *raw){
    HH_SensorReading_t out = *raw;
    /* Reject grossly non-physiological values. (Keeps real faults like 38.5/88.0.) */
    if(out.temp_c   < 20.0f || out.temp_c   > 45.0f) out.valid = false;
    if(out.spo2_pct < 50.0f || out.spo2_pct > 100.0f) out.valid = false;
    return out;
}
```

Read that comment carefully — it's a contract in disguise. FILTER's job is to drop *garbage*
(a disconnected sensor reading 200°C), **not** to drop *bad news*. 38.5°C is a real fever and 88%
is real hypoxia; both must survive FILTER so ALERT can flag them. A filter that's too aggressive is
indistinguishable from a broken alarm.

**ALERT** (`firmware/src/alert/hh_alert.c`) consumes `procQ`, scores against live thresholds,
produces `telemetryQ`:

```c
HH_AlertLevel_t HH_Alert_Evaluate(const HH_SensorReading_t *r){
    float temp_high, spo2_crit;
    HH_Config_GetThresholds(&temp_high, &spo2_crit);   // live values from CONFIG (shared state)
    if(!r->valid)               return HH_ALERT_NONE;   // invalid in -> no alert out
    if(r->spo2_pct < spo2_crit) return HH_ALERT_CRITICAL;   // critical wins
    if(r->temp_c  >= temp_high) return HH_ALERT_HIGH;
    return HH_ALERT_NONE;
}
```

Note the first line of logic: **`if(!r->valid) return HH_ALERT_NONE;`**. ALERT trusts FILTER's
verdict completely — if FILTER marked a reading invalid, ALERT stays silent. That trust is exactly
why FILTER's "keep real faults" rule matters, and it's the thing you're about to prove on Renode.

**Checkpoint 3.** In your own words (one or two sentences): trace the 38.5°C reading from
`HH_Sensor_Read` to a `HIGH` line on the UART, naming the two queues it crosses.

### 3b. Change your stage, and let Renode judge it

Here's the loop this whole course runs on. You'll make a small change, and instead of *hoping* it's
right, you'll ask Renode. We'll use a deliberately instructive mistake first, then the real point.

Open `firmware/src/filter/hh_filter.c` and add one over-eager line to `HH_Filter_Apply` — the kind
of "extra safety" that feels responsible and is actually a bug:

```c
    if(out.temp_c < 20.0f || out.temp_c > 45.0f) out.valid = false;
    if(out.temp_c > 38.0f) out.valid = false;   /* "clamp anything feverish" -- WRONG: eats the real 38.5 */
    if(out.spo2_pct < 50.0f || out.spo2_pct > 100.0f) out.valid = false;
```

Rebuild and run the gate:

```bash
make -C firmware
renode-test tests/system.robot
```

It **compiles cleanly** — the compiler has no idea anything is wrong, because nothing is wrong
*syntactically*. But the suite fails:

```
Injected Faults Raise Correct Alerts    | FAIL |
...
Tests finished with errors :(
```

Read what just happened, because it's the whole lesson: your over-clamp marked the 38.5 reading
`valid = false`; ALERT saw `!r->valid` and returned `NONE`; the HIGH alert never reached the UART;
the test that knows 38.5 *must* raise HIGH caught it. **A behavioral bug the compiler waved through,
Renode stopped — on your laptop, before a single commit.** That is the gate earning its keep.

Now delete that line, rebuild, and confirm you're green again:

```bash
make -C firmware
renode-test tests/system.robot        # -> Tests finished successfully :)
```

That round trip — change, test on Renode, read the failure, fix, test again — is the develop loop.
When you build your *real* subsystem this week, you'll run it constantly, not once at the end.

**Checkpoint 4.** You made FILTER fail the injected-fault test, explained *why* in terms of
`valid` and ALERT's `!r->valid` guard, then restored it to green — all locally.

---

## Part 4 — Commit only when Renode is green (your personal gate)

Now the workflow. You never work on `main`, and you never commit red. For this week's task:

```bash
git checkout -b week-05-<your-subsystem>     # e.g. week-05-filter
# ... build your module; run `renode-test tests/system.robot` as you go ...

make -C firmware && make -C firmware TEST=1  # both variants build
renode-test tests/system.robot               # both suites green
renode-test tests/integration.robot          # ("Tests finished successfully :)")

git add -A
git commit -m "week5: implement FILTER validation; local Renode green"
git push -u origin week-05-<your-subsystem>
```

Then open a **Pull Request** on GitHub (`main` ← your branch). Do not merge yet — Part 5 is what
makes the merge trustworthy.

> **Optional but recommended — make "green before push" automatic.** A git *pre-push hook* runs a
> command before every `git push` and aborts the push if it fails. Drop this in
> `.git/hooks/pre-push` (create the file, make it executable with `chmod +x .git/hooks/pre-push`):
>
> ```bash
> #!/usr/bin/env bash
> # Refuse to push unless the Renode system suite passes locally.
> echo "pre-push: building + running Renode system suite..."
> make -C firmware >/dev/null || { echo "BUILD FAILED — push aborted."; exit 1; }
> renode-test tests/system.robot || { echo "RENODE RED — push aborted."; exit 1; }
> echo "pre-push: green. pushing."
> ```
>
> Now you *can't* push red by accident. The hook is local (it lives in your `.git/`, not in the
> repo), so it's a personal safety net — the team-wide enforcement comes next.

**Checkpoint 5.** You have a `week-05-*` branch pushed and a PR open, and your last local
`renode-test` run was green.

---

## Part 5 — The baseline gate (GitHub Actions + a required check)

Your laptop being green is a promise. The **baseline gate** is what makes the promise enforceable for
everyone — so `main` is *always* a version of HawkHealth that boots and passes on Renode, no matter
who pushed.

The repo already ships the workflow at `.github/workflows/ci.yml`. On every push and every PR it:

1. installs the ARM toolchain and **builds both variants** (`make` and `make TEST=1`);
2. **publishes `hawkhealth.elf`** as a downloadable artifact (so a teammate can grab your exact
   firmware and watch it in Renode without building);
3. installs **the same Renode your laptop runs** and executes **`renode-test`** on
   `tests/system.robot` and `tests/integration.robot`;
4. uploads the test logs.

That is the identical command from Part 2b, run by a robot on a clean machine. If it's green in CI,
it's green for the team — no "works on my machine" escape hatch.

**Make it a *required* check (do this once, in your repo).** A workflow that runs but isn't
*required* is a smoke detector with the battery out. Turn it into a gate:

**Settings → Branches → Add branch protection rule** → Branch name pattern `main` →
tick **Require status checks to pass before merging** → search for and select **`firmware-ci`** →
Save.

Now GitHub will physically block "Merge" on any PR whose CI is red. `main` can no longer go red,
because red can't get in. *That* is "everything runs on Renode before it reaches the baseline,"
enforced.

With the rule on, finish your Part 4 PR the professional way: push → CI runs → the check goes green →
**Merge**. Delete the branch. That branch → PR → CI-green → merge loop is the rhythm for every week
from here.

> **Keeping local and CI in lockstep (the version knob).** The workflow pins the Renode nightly in
> one place, at the top, so there's exactly one line to change when the class moves to a newer nightly
> for the hardware board. In `ci.yml`:
>
> ```yaml
> env:
>   # The exact board-validated nightly from https://builds.renode.io/ — must equal Part 0 locally.
>   RENODE_BUILD: "1.16.1+20260828git00139efee"
> ```
>
> CI downloads `renode-${RENODE_BUILD}.linux-portable-dotnet.tar.gz` from `builds.renode.io` (falling
> back to the plain `linux-portable` build + mono if the dotnet variant isn't published for that
> hash). If you bump `RENODE_BUILD`, everyone re-runs Part 0 with the matching nightly. Local Renode
> and CI Renode move together, always — otherwise a green laptop can still surprise you in CI, which
> is the one thing this whole system exists to prevent.
>
> *One caveat with nightlies:* `builds.renode.io` prunes old dated builds after a while. If CI starts
> failing at the download step with a 404, the pinned nightly was pruned — pick a current one from
> `builds.renode.io`, update `RENODE_BUILD`, and have everyone re-sync locally. (If you want to avoid
> that entirely, cache the tarball as a CI artifact or in your own storage and point the download at
> it.)

**Checkpoint 6.** Your repo has branch protection on `main` requiring `firmware-ci`, and you merged
your Week 5 PR only after the check went green.

---

## Part 6 — Bringing a classmate's subsystem online

Here's the part that makes seven people into one system. **You already have the whole system** — your
template repo contains all seven subsystems, so it always builds and always runs. For the stages you
don't own, you're running the template's version. As each owner finishes *their* real
implementation, you pull it in and re-prove the system on Renode.

Say you own FILTER, and the SENSOR owner just shipped. To bring their SENSOR online:

1. **Take their implementation, not their header.** Copy their subsystem's `.c` (and any private
   helpers it adds) from their repo into your `firmware/src/sensor/`. **Do not** touch
   `firmware/include/hh_sensor.h` — the header is the *frozen contract*, identical in both repos. You
   are swapping an implementation *behind* a contract, not changing the contract.

   ```bash
   # from your clone root, with their repo cloned next door:
   cp ../hawkhealth-<their-name>/firmware/src/sensor/hh_sensor.c firmware/src/sensor/hh_sensor.c
   ```

2. **Rebuild and run the gate — locally, first, as always:**

   ```bash
   make -C firmware && renode-test tests/system.robot
   ```

3. **Read the result as an integration verdict:**
   - **Green** → their SENSOR honors the frozen contract; integration proven, in your repo, on
     Renode. Commit it on a branch (`git commit -m "integrate SENSOR from <name>"`), PR, let CI
     re-confirm, merge.
   - **Won't compile / red** → the contract was violated somewhere (a changed signature, a queue
     type mismatch, a wrong `sizeof`). That's not your bug to silently patch — it's a conversation
     with the owner, with the failing `renode-test` output as the evidence. This is *exactly* why the
     interface spec froze in Week 3: the header is the promise, and Renode is where a broken promise
     shows up.

Because every repo proves the whole system in CI, integration happens continuously, all semester —
there is no terrifying big-bang merge at the end. The frozen contract is what lets a file drop in;
Renode is what confirms it did.

**Checkpoint 7.** You can explain why you copy a neighbor's `.c` but never their `.h`, and what a
red `renode-test` after an integration is telling you.

---

## Exit criteria

- [ ] `renode --version` shows the class-standard nightly (`1.16.1+20260828git00139efee`), matching
      CI's `RENODE_BUILD`; both firmware variants build locally.
- [ ] You've *watched* telemetry in Renode and seen the 38.5→HIGH and 88.0→CRITICAL injected alerts.
- [ ] You've run **both** robot suites locally to `Tests finished successfully :)`.
- [ ] You drove FILTER red on the injected-fault test, explained it via `valid` / `!r->valid`, and
      restored green — locally, before committing.
- [ ] Your repo enforces `firmware-ci` as a **required** check on `main`, and you merged your Week 5
      PR only after CI went green.
- [ ] You can describe how you'd pull in a classmate's subsystem and prove the integration on Renode.

## What to submit

1. A **screenshot of the Renode `usart3` window** showing the injected `HIGH` (38.5) and `CRITICAL`
   (88.0) alert lines.
2. A **screenshot or paste of `renode-test tests/system.robot` going red** on the FILTER over-clamp,
   and **green** after you removed it — the local gate catching a behavioral bug.
3. The **link to your merged Week 5 PR**, showing the green `firmware-ci` check.
4. A **screenshot of your `main` branch-protection rule** with `firmware-ci` required.
5. **Two or three sentences:** why is "green on Renode locally" a stronger promise than "it compiles,"
   and why must your local Renode and CI's Renode be the same version?
6. Your **AI Interaction Log** entry for Week 5.

## What's next

You now have the machine that will carry every future week: build, prove on Renode, gate the
baseline. **Next week** you put a real subsystem through it under pressure — the scheduler starts
making *choices*, and a task that never yields can starve the ones beneath it. Same loop, harder
question: the system won't crash, it'll just quietly stop keeping up — and Renode will show you
where.
