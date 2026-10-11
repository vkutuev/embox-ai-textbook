# GPIO interrupt support on K1921VG1T

Findings from verifying the IRQ variant of the LED/button counter
example on the NIIET-DEV-K1921VG1T board (2026-10-10 / 2026-10-12).
Full evidence: the board guide's experiment journal.

## The working recipe

The IRQ variant now works (verified: five consecutive presses →
`counter = 1..5`, no interrupt storm, console stays alive). The key is
**not to re-run the driver's `gpio_setup_mode()`** once the interrupt
is configured: every such call re-writes the edge-configuration
registers (`INTTYPESET`/`INTEDGECLR`/`INTPOLCLR`), which latches a
spurious `INTSTATUS` bit on this silicon. With `INTSTATUS` set and the
interrupt enabled, the line stays asserted and the PLIC re-pends after
every claim — an interrupt storm.

Instead, mask, clear, and re-arm by writing only the GPIO interrupt
registers directly (see `led_counter_irq.c`):

- mask   → `GPIOB_INTENCLR = pin`
- clear  → `GPIOB_INTSTATUS = pin` (write-1-to-clear, per the UM)
- re-arm → clear `INTSTATUS`, then `GPIOB_INTENSET = pin`

This never touches the edge-configuration registers, so no spurious
latch occurs.

## 0001 — niiet_gpio: record the attached IRQ handler (real driver bug)

`niiet_gpio_setup_irq()` checks `niiet_gpio_priv.gpio_irq_handler[port]`
to decide whether the PLIC IRQ is already attached, but the array is
never filled in. Every `gpio_setup_mode()` call therefore re-runs
`irq_attach()`, which fails with `-EBUSY`, and the function returns
*before* enabling the interrupt — silently leaving the pin masked.

This patch fixes that. It is not strictly required by the example above
(which calls `gpio_setup_mode()` once), but it is a real bug that bites
any code that configures the same GPIO port more than once (e.g. the
`gpio_button` driver).

## PLIC start/end handshake (debugging note)

The claim register (`0xFE200004`) uses a start/end protocol: reading it
"starts" the interrupt and holds the target's EIP low until a write of
the same value ("end") completes it. Reading it from a debugger without
the matching write leaves the target unable to take further external
interrupts. The Embox handler writes the "end" (`irqctrl_eoi`), so this
only matters when poking the register manually.
