# Week 2a Lab — STM32 Dev-Board & MCU Architecture

> **Mentor's note.** Last week you saw tasks *run*. This week you look *under* them — at the
> chip itself: where code lives, where data lives, how the CPU knows what to do when it powers
> on, and how "talking to a peripheral" is really just reading and writing memory. The clever
> part: you don't need special tools to see any of this. It's all sitting in the HawkHealth repo
> you already cloned. We'll read the real files, then feel a real input on the board in Wokwi.

**By the end of this lab you will be able to:** point to the F767's memory map, its reset
vector, and a peripheral register — *in HawkHealth's own source* — and drive a GPIO input on a
board in Wokwi.

> Open the HawkHealth repo in your editor. Any editor works; **VS Code** is a nice default for a
> C + Makefile + git project (and it has a Wokwi extension for later). CubeIDE arrives when we
> hit real hardware — you don't need it yet.

---

## Part 1 — The MCU, in the actual code (Chapter 2, made real)

Lecture gave you the architecture. Now find each piece in HawkHealth. This is the whole point:
the abstract MCU diagram *is* these files.

### 1a. The memory map — where things live

Open **`firmware/src/startup/stm32f767.ld`** (the linker script). Find the `MEMORY` block:

```
FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
```

That's the F767's map in two lines: **2 MB of flash starting at `0x08000000`** (your code and
constants — read-only, `rx`) and **512 KB of RAM at `0x20000000`** (variables and stacks —
`rwx`). Every address the CPU touches falls somewhere in this map.

> **Compare to the C031C6 you used in Wokwi:** 32 KB flash, 12 KB RAM. Same *scheme* (flash low,
> RAM at `0x20000000`), 40× less of it. That's the F767 being a much bigger chip — same family,
> same ideas.

**Checkpoint 1a.** You can state where flash begins and where RAM begins on the F767.

### 1b. The vector table — what runs at power-on

Open **`firmware/src/startup/startup_stm32f767.c`** and find `g_vectors[]`. The first two entries
are the most important thing in the whole file:

```c
(void(*)(void))(&_estack),   // [0] initial stack pointer
Reset_Handler,               // [1] the first code that runs
```

On reset, the Cortex-M7 does exactly one thing automatically: it loads the **stack pointer** from
address `0x08000000` and **jumps to the reset vector** at `0x08000004`. `Reset_Handler` then sets
up memory and calls `main()`. Below those, find the **system exception handlers** the RTOS needs:

```c
SVC_Handler, ... PendSV_Handler, SysTick_Handler,
```

Those three are the hardware hooks FreeRTOS uses to switch tasks (`PendSV`), start the first task
(`SVC`), and keep time (`SysTick`). The NVIC and SysTick from lecture aren't abstractions — they're
these vector slots.

**Checkpoint 1b.** You can point to the reset vector and to `SysTick_Handler` in the vector table.

### 1c. Peripherals are just memory — memory-mapped I/O

Open **`firmware/src/platform/hh_platform.c`** and look at the top:

```c
#define USART3_TDR   REG(0x40004828u)   // send a byte by WRITING this address
#define GPIOB_ODR    REG(0x40020414u)   // the LED pins live here
```

Sending a character over UART is *writing to a memory address*. Toggling the LED is *flipping a
bit at another address*. There's no magic "UART object" — the peripheral **is** a block of memory
the hardware watches. This is the single most important idea in Chapter 2.

**Checkpoint 1c.** You can name one peripheral register address in `hh_platform.c` and say what
writing to it does.

---

## Part 2 — Feel a real input on the board (Wokwi)

Reading is half of it; now drive a pin. You'll add a **push button** (a GPIO *input*) and have a
FreeRTOS task react to it — the input side of the board, which HawkHealth's COMMAND subsystem will
need for real later.

1. In Wokwi: **New Project → STM32 → ST Nucleo C031C6** (same as Week 1).
2. Paste **[`wokwi/week02a/sketch.ino`](../wokwi/week02a/sketch.ino)**.
3. **Library Manager → + Add → `STM32duino FreeRTOS`** (same library as Week 1).
4. In the **diagram**, add a **pushbutton**: one leg to pin **PB2**, the diagonal leg to **GND**.
   (The sketch uses the chip's internal pull-up, so the pin reads HIGH until the button pulls it
   LOW.)
5. Press **▶ Play**. The on-board LED (PA5) blinks steadily. **Click the button** — the Serial
   Monitor prints `BUTTON pressed` each time.

**What's happening (and why it matters):** one task blinks; another task *polls* the button pin
every 20 ms and prints on a fresh press. Two independent jobs, neither blocking the other — the
RTOS again — plus your first GPIO **input**.

> **Polling vs. interrupts — a preview.** We *poll* the button here (check it on a schedule)
> because it's simple and reliable in the simulator. On real hardware you'll instead wire the
> button to an **EXTI interrupt** through the **NVIC** — the same NVIC whose vector slots you
> found in Part 1b — so the CPU is notified the instant it's pressed, with no polling. That's
> Week 8. Today, just feel the input working.

**Checkpoint 2.** Clicking the button prints `BUTTON pressed` in the Serial Monitor while the LED
keeps blinking.

---

## Exit criteria

- [ ] You can point — in HawkHealth's own files — to the **flash origin**, the **reset vector**,
  and a **peripheral register**.
- [ ] Your Wokwi project blinks an LED *and* reacts to a button press.

## What to submit (weekly milestone)

1. A **screenshot** of your Wokwi button project (Serial Monitor showing `BUTTON pressed`).
2. Three one-line answers: **where does flash start**, **what's at `0x08000004`**, and **what does
   writing `USART3_TDR` do** — each with the file you found it in.
3. Your **AI Interaction Log** entry for Week 2a.

## What you are *not* doing yet

Still no changes to the HawkHealth firmware, no Renode, no board. Reading the architecture and
feeling GPIO is the work. Next week: the developer toolchain in depth, and your interface-spec
draft.
