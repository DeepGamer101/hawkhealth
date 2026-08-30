# Week 2b Lab — Dev-Tools

> **Mentor's note.** This week you set up the tools you'll use for the rest of the semester and
> run the real HawkHealth system on your own machine for the first time. Two installs
> (STM32CubeIDE and Renode), one satisfying payoff (watch the pipeline stream telemetry in the
> emulator), and one deliverable (**your interface-spec section is due**). No firmware changes yet
> — this is about your bench.

**By the end you will have:** CubeIDE and Renode installed, HawkHealth running in Renode from a
CI-built firmware, and your subsystem's interface-spec draft committed.

---

## Part 1 — Install STM32CubeIDE (your editor now, your hardware debugger later)

STM32CubeIDE is the IDE the course and the textbook use. You'll browse and edit HawkHealth in it
now, and use it to flash and debug the real board in Weeks 9+.

1. Download STM32CubeIDE: https://www.st.com/en/development-tools/stm32cubeide.html
   (OS: Windows, version **1.18.1**; make a free MyST account when prompted).
2. Install with default options. First launch: pick a workspace folder.
3. **Open HawkHealth in it:** **File → Open Projects from File System… → Directory…** → select your
   cloned `hawkhealth` folder → Finish. You can now navigate the whole codebase with a real IDE —
   jump to definitions, search symbols, read `firmware/src/…`.

> You are **not** building in CubeIDE this week — the grading loop builds in CI, and local Renode
> runs use the CI-built firmware (Part 3). CubeIDE here is your reading/editing tool; its build +
> debug role arrives with real hardware.

**Checkpoint 1.** CubeIDE opens and you can browse `firmware/src/` and open `hh_sensor.c`.

---

## Part 2 — Install Renode (the Stage-2 emulator)

Renode runs the actual STM32F767 firmware with no board. You'll use it heavily in Weeks 5–8.

1. Go to **https://builds.renode.io/** and download the latest
   **`renode-*.windows-portable-dotnet.zip`** (a nightly — it ships the models we need).
2. Unzip to a simple path, e.g. `C:\renode\`. No installer; it's a self-contained folder.
3. Launch **`Renode.exe`** from that folder. A **Monitor** window opens with a `(monitor)` prompt.

> Why a nightly and not the stable release: the stable build predates some of the STM32F7
> peripheral models. The nightly ships them. (Same reason from the setup guide.)

**Checkpoint 2.** Renode launches and shows the `(monitor)` prompt.

---

## Part 3 — Run HawkHealth in Renode (first local run of the real system)

You don't build locally — you fetch the firmware your **CI already built**, then run it.

1. **Get the firmware from CI.** On your repo's GitHub page → **Actions** tab → click the most
   recent green run → scroll to **Artifacts** → download **`hawkhealth-firmware`**. It arrives as a
   **zip** — you must **unzip it** to get **`hawkhealth.elf`** as a real file. Then note its exact,
   full path (right-click → Properties, or copy it from Explorer's address bar).
   > If `LoadELF` later errors with *"Parameters did not match the signature,"* the path is wrong —
   > the file is still zipped, is one folder deeper, or the name is off (turn on "File name
   > extensions" in Explorer to check it's not `hawkhealth.elf.elf`).
   > *Dev-tools lesson:* CI doesn't just test — it **builds and publishes artifacts**. This `.elf`
   > is the exact binary that passed the tests.
2. **Edit the run script.** Open `renode/hawkhealth.resc` from your clone in a text editor. Change
   the **two paths** to absolute paths on your machine (forward slashes, no quotes):
   - the platform file: `<your-clone>/platforms/hawkhealth_f767.repl`
   - the firmware you just downloaded: `<...>/hawkhealth.elf`
   *(The script loads the ELF on a plain `sysbus LoadELF` line — no macro block to fuss with.)*
3. **Run it.** In the Renode Monitor, `include` the **run script** (`renode/hawkhealth.resc`) —
   **not** the platform `.repl` file. The `.resc` is what loads *both* the hardware model and your
   firmware; the `.repl` alone loads no firmware (you'll see `PC = 0x0, SP = 0x0` and a flood of
   "non existing peripheral" warnings — that means no `.elf` was loaded).
   ```
   Clear
   include @C:/path/to/your/hawkhealth/renode/hawkhealth.resc
   start
   ```
   (`include`, not `install`; point at the **.resc**; `@` prefix; forward slashes; no quotes.)
4. A **`hawkhealth:sysbus.usart3`** window opens. You should see the telemetry stream:
   ```
   [HawkHealth] system starting
   [HH] t=1000 temp=36.8 spo2=98.0 hr=73 alert=NONE
   [HH] t=2000 ...
   ```
   Let it run ~30 seconds and you'll see the injected faults become alerts:
   `... temp=38.5 ... alert=HIGH` (call 300) and `... spo2=88.0 ... alert=CRITICAL` (call 500).

> Blank USART window or an error? It's almost always a path (use absolute, forward slashes, no
> quotes) or that you forgot `start`. If it's stuck, `Clear` and re-`include`.

**Checkpoint 3.** The `usart3` window streams `[HH] t=… temp=… alert=…` lines — HawkHealth running
on the emulated F767, on your machine.

---

## Part 4 — Your interface-spec draft (**due this week**)

You own one subsystem. This week you author its section of the frozen interface spec.

1. Open **`docs/interface-spec.md`** and find your subsystem's section (S1–S7).
2. Fill in the **Owner:** line with your name, and flesh out your subsystem's contract: what each
   public function promises, what it consumes/produces, and any invariants (e.g., "call `Init`
   before the first read"). Match the actual header in `firmware/include/hh_<name>.h`.
3. Commit on a branch and open a **pull request** (`git`/GitHub Desktop):
   - branch: `spec-draft-<yourname>`, commit message `Interface spec draft — <SUBSYSTEM>`
   - open the PR; confirm **CI stays green** (you only edited a doc).

> This is a *draft*. In Week 4 the spec **freezes** — after that, changing a public signature needs
> a spec revision, because everyone codes against it.

**Checkpoint 4.** Your spec section is filled in, pushed on a PR, and CI is green.

---

## Exit criteria

- [ ] CubeIDE installed; you can browse HawkHealth in it.
- [ ] Renode installed; HawkHealth's telemetry streams in the `usart3` window.
- [ ] Your interface-spec section is drafted and pushed on a PR (CI green).

## What to submit (weekly milestone)

1. A **screenshot** of the Renode `usart3` window showing an `alert=HIGH` or `alert=CRITICAL` line.
2. The **link to your interface-spec PR**.
3. Your **AI Interaction Log** entry for Week 3.

## What you are *not* doing yet

No firmware changes, no hardware. You built your bench and ran the system. Next week: super-loops
(and the spec freezes).
