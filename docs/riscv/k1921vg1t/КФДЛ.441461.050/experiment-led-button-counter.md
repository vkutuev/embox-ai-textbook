# Experiment: LED/button counter (2026-10-10 / 2026-10-12)

Hardware verification of the
[LED/button counter example](example-led-button-counter.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 UART at `/dev/ttyUSB1`).

## Verified board facts

| Item | Result | How |
| ---- | ------ | --- |
| LED pins | board **LED1..LED4** = GPIOC[12..15], **active-high** (pin high = LED on) | `pin GPIOC 12 set` lights the board's LED1; walking-1 confirmed the order |
| LED polarity vs BSP | The SDK BSP implies active-low (`BSP_LED_On()` clears `DATAOUT`); the real board is active-high — trust the empirical result | — |
| Button | GPIOB[3], **press = low**, idle high | `pin GPIOB 3 get` reads high idle / low pressed |
| Floating LED pins | Pins left as inputs float high and light their LEDs — drive all four pins explicitly | observed when only 2 of 4 pins were driven |

## Variant results

### Polling — works

`led_counter_poll` increments once per press, prints the value, shows it
in binary on the LEDs, wraps at 15. Verified with many presses.

### IRQ — works, after finding the latch trigger (2026-10-12)

First version wedged the system on press. The investigation found a
chain of issues; the decisive one is a **spurious `INTSTATUS` latch
caused by re-running the driver's configuration**:

1. **`niiet_gpio` never records the attached IRQ handler.** The guard
   `if (NULL == niiet_gpio_priv.gpio_irq_handler[irq_port])` is always
   true, so every `gpio_setup_mode()` re-runs `irq_attach()`; the
   second call fails with `-EBUSY` and returns *before* enabling the
   interrupt. Net effect: after two `gpio_setup_mode()` calls on the
   same port the pin interrupt is silently masked forever. Fixed with
   one line (patch `0001`).
2. **Every `gpio_setup_mode()` re-writes the edge-configuration
   registers** (`INTTYPESET`/`INTEDGECLR`/`INTPOLCLR`), which latches a
   spurious `INTSTATUS` on this silicon. With the interrupt enabled,
   `INTSTATUS` set means the line stays asserted and the PLIC re-pends
   after every claim — an interrupt storm. An earlier ISR that masked
   via `gpio_setup_mode()` therefore *fed* the storm: GDB showed
   `btn_events` reach 0x40E4A (~266k ISR calls) from one press, and the
   stack growth zeroed `.data` globals (`btn_events` read 0 while the
   ISR demonstrably ran).
3. The RISC-V trap handler nests interrupts during dispatch
   (`ipl_enable()`), which turned the storm into stack corruption (dead
   UART RX thread = dead console). This is an amplifier, not the
   trigger: with the latch eliminated, the stock handler is fine.

**Fix** (in `led_counter_irq.c`): after the initial configuration, never
call `gpio_setup_mode()` again. Mask, clear, and re-arm by writing only
the GPIO interrupt registers directly — `INTENCLR`, `INTSTATUS`
(write-1-to-clear, which works per the UM), then `INTENSET`. This never
touches the edge-configuration registers, so no spurious latch.

**Verified**: five consecutive presses → `counter = 1..5`, no storm,
console alive, interrupt re-armed between presses.

> Note: an intermediate conclusion "`INTSTATUS` write-1-to-clear does
> not work on this silicon" was wrong — those observations were the
> configuration-write re-latch (and some failed debug-module writes).
> The UM's write-1-to-clear is correct.

### Input subsystem (gpio_button) — breaks every boot

With `embox.driver.input.button.gpio_button(pin_port=1, pin_num=3)`
enabled, the boot hangs at `initializing embox.driver.input.button.gpio_button`
(runlevel 0): the driver configures a both-edges
(`RISING_FALLING`) interrupt on the idle-high button pin, `INTSTATUS`
latches at enable, and the PLIC storms before the button handler is
attached. The boot only continued after the pending interrupt was
claimed externally (a GDB read of the PLIC claim register
`0xFE200004` returned 9 and unblocked the boot). The same
mask/enable-only discipline would apply, but `gpio_button` would need a
driver change (re-arm on a timer) — not done yet.

## Root cause notes

- The GPIO `INTSTATUS` flag is set by the edge configuration and is
  independent of `INTENSET` (UM chapter 15); in both-edges mode
  `INTPOLSET` is ignored. Writing 1 clears the flag (UM); the flag is
  not auto-cleared.
- On this silicon, writing the edge-configuration registers latches a
  spurious `INTSTATUS`; a both-edges configuration on an idle-high pin
  latches at enable (the `gpio_button` boot storm).
- The RISC-V trap handler nests interrupts during dispatch. A
  permanently-asserted GPIO line therefore causes unbounded nesting and
  corrupts unrelated kernel state (the UART RX drain lthread).
- `niiet_gpio` has a bookkeeping bug (missing
  `gpio_irq_handler[port]` assignment) that makes repeated
  `gpio_setup_mode()` calls silently disable the interrupt.
- The PLIC claim register (`0xFE200004`) uses a start/end handshake:
  reading it "starts" the interrupt and holds the target's EIP low
  until a write of the same value ("end"). Reading it from a debugger
  without the matching write looks like a stuck interrupt (`MEIP=0`
  while pending). This is a debugging artifact, not a board fault.

## Environment notes

- The CH340 node re-enumerated as `/dev/ttyUSB1` (was `ttyUSB0` in the
  previous session); `stty -F /dev/ttyUSB1 115200 raw -echo` before
  capture.
- After interrupt storms, the debug module desynchronizes: OpenOCD
  reports `Unsupported DTM version: -1`; also `LIBUSB_ERROR_BUSY` when
  a stale OpenOCD instance still holds the J-Link. Kill all OpenOCD
  processes and retry; a power-cycle may be needed.
- The flashed image with `led_counter_poll` + `led_counter_irq` boots
  cleanly; `gpio_button` was removed from the build for these tests.
