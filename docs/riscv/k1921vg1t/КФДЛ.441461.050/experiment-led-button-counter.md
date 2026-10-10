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

### IRQ — wedges the system on press

`led_counter_irq` (falling-edge IRQ) starts fine and idles fine, but a
button press wedges the system: output freezes (banner truncated
mid-print), no counter output, and afterwards the shell is permanently
deaf (UART RX never drained again until reset).

Observed via JTAG/GDB:

- During/after the press: repeated traps with `mcause=0x8000000B`
  (machine external interrupt), `mepc` at the idle instruction.
- After release the system partially recovers: `INTSTATUS=0`, PLIC
  pending shows only the healthy TMR0 source (16), the core runs the
  idle task and services timer interrupts — but the UART RX interrupt
  (PLIC source 64) stays asserted (`MIS=0x50`, `DR` unread) and the RX
  drain thread never runs, so the console is dead.
- The pressed pin re-asserts the GPIO line faster than the interrupt
  handler clears it: `INTSTATUS` write-1-to-clear does not take effect
  immediately on this silicon (see below), and
  `riscv_interrupt_handler()` enables interrupts during dispatch
  (`ipl_enable()`), so the storm nests without bound and corrupts the
  RX thread's state.

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
- Writing 1 to `INTSTATUS` should clear the flag (UM), but on this
  silicon the clear does not take effect immediately in the tested
  modes, so an asserted line keeps re-pending the PLIC source.
- The RISC-V trap handler nests interrupts during dispatch. A
  re-asserting GPIO line therefore causes unbounded nesting and
  corrupts unrelated kernel state (the UART RX drain lthread).
- Consequences for the textbook: the polling variant is the verified
  one. Making the IRQ/input variants work needs driver fixes
  (immediate INTSTATUS clearing, no nested dispatch), not example-code
  changes.

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
