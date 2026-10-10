# Experiment: LED/button counter (2026-10-10)

Hardware verification of the
[LED/button counter example](example-led-button-counter.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 UART at `/dev/ttyUSB1`).

## Verified board facts

| Item | Result | How |
| ---- | ------ | --- |
| LED pins | LED0..LED3 = GPIOC[12..15], **active-high** (pin high = LED on) | `pin GPIOC 12 set` lights LED0; walking-1 test confirmed the order |
| LED polarity vs BSP | The SDK BSP implies active-low (`BSP_LED_On()` clears `DATAOUT`); the real board is active-high — trust the empirical result | — |
| Button | GPIOB[3], **press = low**, idle high | `pin GPIOB 3 get` reads high idle / low pressed |
| Floating LED pins | Pins left as inputs float high and light their LEDs — drive all four pins explicitly | observed when only 2 of 4 pins were driven |

## Variant results

### Polling — works

`led_counter_poll` increments once per press, prints the value, shows it
in binary on the LEDs, wraps at 15. Verified with many presses.

### IRQ — blocked by a silicon latch + driver bugs (root cause found)

`led_counter_irq` (falling-edge IRQ) starts fine, but pressing the
button wedges the system. Follow-up session (same day, with a driver
fix applied) traced the full chain and found three separate problems:

1. **`niiet_gpio` never records the attached IRQ handler.** The guard
   `if (NULL == niiet_gpio_priv.gpio_irq_handler[irq_port])` is always
   true, so every `gpio_setup_mode()` re-runs `irq_attach()`; the
   second call fails with `-EBUSY` and returns *before* enabling the
   interrupt. Net effect: after two `gpio_setup_mode()` calls on the
   same port the pin interrupt is silently masked forever. Fixed with
   one line (patch `0001`, verified: `INTENSET` now stays set and the
   interrupt chain works up to the user handler).
2. **`INTSTATUS` does not clear on this silicon.** While pressed (and
   even after release), `INTSTATUS` stays latched at `0x8`; neither
   write-1-to-clear (the UM procedure) nor write-0 clears it. The
   asserted line keeps re-pending the PLIC, so the trap handler is
   re-entered forever. GDB evidence: `btn_events` reached 0x40E4A
   (~266k ISR calls) from a single press, with `INTSTATUS=8` still
   latched after release.
3. **The RISC-V trap handler nests interrupts during dispatch**
   (`ipl_enable()` in `riscv_interrupt_handler()`). With the line
   permanently asserted, each re-entry pushes a trap context on the
   interrupted thread's stack — unbounded growth that smashes
   unrelated kernel state: the UART RX drain thread dies (console
   dead) and `.data` globals are overwritten (`btn_events` read 0
   while the ISR demonstrably ran). Patching out the nesting
   (patch `0002`) bounds the storm: the system survives, the counter
   counts once, but the CPU spins at 100% while the line is asserted.

Conclusion: the polling variant is the verified one. Interrupt-driven
buttons on this board need the two patches *plus* an understanding of
the `INTSTATUS` latch behavior (NIIET errata/manual); the patches are
in `examples/.../led_button_counter/patches/`.

### Input subsystem (gpio_button) — breaks every boot

With `embox.driver.input.button.gpio_button(pin_port=1, pin_num=3)`
enabled, the boot hangs at `initializing embox.driver.input.button.gpio_button`
(runlevel 0): the driver configures a both-edges
(`RISING_FALLING`) interrupt on the idle-high button pin, `INTSTATUS`
latches at enable, and the PLIC storms before the button handler is
attached. The boot only continued after the pending interrupt was
claimed externally (a GDB read of the PLIC claim register
`0xFE200004` returned 9 and unblocked the boot).

## Root cause notes

- The GPIO `INTSTATUS` flag is set by the edge configuration itself
  and is independent of `INTENSET` (UM chapter 15); in both-edges mode
  `INTPOLSET` is ignored.
- On this silicon `INTSTATUS` cannot be cleared reliably by the
  documented write-1 while the edge configuration keeps the condition
  asserted — the interrupt line stays asserted and re-pends the PLIC
  after every claim (observed in both-edges and falling-edge modes).
- The RISC-V trap handler nests interrupts during dispatch. A
  permanently-asserted GPIO line therefore causes unbounded nesting
  and corrupts unrelated kernel state (the UART RX drain lthread).
- `niiet_gpio` has a bookkeeping bug (missing
  `gpio_irq_handler[port]` assignment) that makes repeated
  `gpio_setup_mode()` calls silently disable the interrupt.
- Consequences for the textbook: the polling variant is the verified
  one. Making the IRQ/input variants work needs the two patches in
  `examples/.../led_button_counter/patches/` plus a silicon-level
  understanding of the `INTSTATUS` latch.

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
