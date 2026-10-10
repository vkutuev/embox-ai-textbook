# Patches for GPIO interrupt support on K1921VG1T

Two Embox patches discovered while verifying the IRQ variant of the
LED/button counter example on the NIIET-DEV-K1921VG1T board
(2026-10-10). Full evidence: the board guide's experiment journal.

## 0001 — niiet_gpio: record the attached IRQ handler

`niiet_gpio_setup_irq()` checks `niiet_gpio_priv.gpio_irq_handler[port]`
to decide whether the PLIC IRQ is already attached, but the array is
never filled in. Every `gpio_setup_mode()` call therefore re-runs
`irq_attach()`, which fails with `-EBUSY`, and the function returns
*before* enabling the interrupt — silently leaving the pin masked.

Symptom: after any second `gpio_setup_mode()` on the same port, the
GPIO interrupt never fires (`INTENSET` stays clear), even though the
pin, `INTSTATUS`, and the PLIC all look fine.

## 0002 — riscv: do not nest interrupt dispatch

`riscv_interrupt_handler()` calls `ipl_enable()` around
`irq_dispatch()`. When an asserted GPIO line re-pends the PLIC faster
than the handler clears it, every re-entry pushes another trap context
on the interrupted thread's stack. With the K1921VG1T GPIO quirk
(`INTSTATUS` stays latched), nesting is unbounded and the stack
smashes unrelated kernel state (the UART RX drain thread and `.data`
globals), killing the console.

Disabling the nested dispatch bounds the storm: the system survives a
button press (the counter still counts), though it spins at 100% while
the line stays asserted.

## Known remaining issue

Neither patch fixes the underlying latch: on this silicon,
`INTSTATUS` cannot be cleared reliably by the documented write-1 while
the edge configuration keeps the condition asserted, so the interrupt
line stays asserted and the CPU keeps servicing it. Until that is
understood (NIIET errata/manual), interrupt-driven buttons on this
board remain unusable; use the polling variant.
