# GPIO interrupt support on K1921VG1T

Findings from verifying the IRQ variant of the LED/button counter
example on the NIIET-DEV-K1921VG1T board (2026-10-10 / 2026-10-12).
Full evidence: the board guide's experiment journal.

## The working recipe

The IRQ variant now works (verified: five consecutive presses →
`counter = 1..5`, no interrupt storm, console stays alive). The recipe is
to mask the pin on the first edge and never let the interrupt re-enter
during a burst of button bounce.

Mask, clear, and re-arm by writing only the GPIO interrupt registers
directly (see `led_counter_irq.c`):

- mask   → `GPIOB_INTENCLR = pin`
- clear  → `GPIOB_INTSTATUS = pin` (write-1-to-clear, per the UM)
- re-arm → clear `INTSTATUS`, then `GPIOB_INTENSET = pin`

An earlier hypothesis — that re-running `gpio_setup_mode()` latches a
spurious `INTSTATUS` by re-writing the edge-configuration registers —
was tested with a register-level probe and **not reproduced**: those
writes do not latch `INTSTATUS` on their own. The storm is best
explained by button bounce amplified by the RISC-V trap handler's nested
dispatch; masking on the first edge sidesteps it regardless.

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

## 0002 — gpio_button: drop GPIO_MODE_INT_SECTION (level-trigger bug)

`gpio_button` passed `GPIO_MODE_INT_SECTION | GPIO_MODE_INT_RISING_FALLING`.
`GPIO_MODE_INT_SECTION` is `0xff000000`, which overlaps every individual
interrupt-mode bit, so `niiet_gpio_setup_irq()` decoded the mode as a
**level** trigger with **rising** polarity on the idle-high pin. That
asserts `INTSTATUS` permanently and storms the PLIC, hanging the boot at
`initializing embox.driver.input.button.gpio_button`.

The patch drops `GPIO_MODE_INT_SECTION` and keeps the both-edges
`RISING_FALLING` mode, which now decodes to an **edge** trigger (both
edges) — it only fires on real transitions, so the boot is fixed and
press/release are both still reported. (The handler's
`USER_BUTTON_PRESSED` polarity is still inverted for an active-low
button — a separate, unfixed issue.)

## PLIC start/end handshake (debugging note)

The claim register (`0xFE200004`) uses a start/end protocol: reading it
"starts" the interrupt and holds the target's EIP low until a write of
the same value ("end") completes it. Reading it from a debugger without
the matching write leaves the target unable to take further external
interrupts. The Embox handler writes the "end" (`irqctrl_eoi`), so this
only matters when poking the register manually.
