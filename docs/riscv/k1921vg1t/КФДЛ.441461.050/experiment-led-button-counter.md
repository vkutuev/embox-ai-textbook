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

### IRQ — works (2026-10-12)

First versions wedged the system on press. Two concrete issues were
found; the exact storm trigger remains partially open (below):

1. **`niiet_gpio` never records the attached IRQ handler.** The guard
   `if (NULL == niiet_gpio_priv.gpio_irq_handler[irq_port])` is always
   true, so every `gpio_setup_mode()` re-runs `irq_attach()`; the
   second call fails with `-EBUSY` and returns *before* enabling the
   interrupt — the pin interrupt is silently masked. Fixed with one
   line (patch `0001`, verified with `niiet_gpio_check`).
2. **Interrupt storm on press.** Repeated presses produced an unbounded
   storm: GDB showed `btn_events` reach 0x40E4A (~266k ISR calls) from
   one press, and the stack growth zeroed `.data` globals (dead UART RX
   thread = dead console).

**Fix** (in `led_counter_irq.c`): mask the pin on the first edge
(`INTENCLR`), then debounce and re-arm (`INTSTATUS` clear + `INTENSET`)
in the main loop. The interrupt never re-enters, so the storm is
prevented regardless of its exact trigger. Verified: five consecutive
presses → `counter = 1..5`, no storm, console alive.

**Storm mechanism — not fully isolated.** An early hypothesis — that
re-running `gpio_setup_mode()` re-writes the edge-configuration
registers and latches a spurious `INTSTATUS` — was tested with a
register-level probe (writing `INTTYPESET`/`INTEDGECLR`/`INTPOLCLR`/
`INTENSET`/`INTENCLR` individually, pin high and low) and **not
reproduced**: no write latched `INTSTATUS`, and `INTSTATUS`
write-1-to-clear works per the UM. The leading remaining explanation is
button bounce generating many edges, amplified by the RISC-V trap
handler's nested dispatch (`ipl_enable()`), which overflows the
interrupted thread's stack. That is an Embox core issue, not a
`niiet_gpio` config bug, and the mask-on-first-edge workaround sidesteps
it.

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

- The GPIO `INTSTATUS` flag is set by the edge logic and is independent
  of `INTENSET` (UM chapter 15); in both-edges mode `INTPOLSET` is
  ignored. Writing 1 clears the flag (UM); the flag is not auto-cleared.
- A register-level probe showed that writing the edge-configuration
  registers (`INTTYPESET`/`INTEDGECLR`/`INTPOLCLR`) or the enable/mask
  registers does **not** latch `INTSTATUS` on its own — the earlier
  "re-config latch" hypothesis was not confirmed. The storm on a pressed
  button is best explained by bounce + the trap handler's nested
  dispatch (see above).
- The `gpio_button` boot storm is a separate, still-unexplained case: a
  both-edges configuration on an idle-high pin latches `INTSTATUS` at
  enable.
- The RISC-V trap handler nests interrupts during dispatch. With a
  re-asserting line (or a burst of bounce edges), this nests without
  bound and corrupts unrelated kernel state (the UART RX drain lthread).
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
