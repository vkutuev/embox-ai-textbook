# Experiment: timer blink (2026-10-13)

Hardware verification of the
[timer blink example](example-timer-blink.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 UART at `/dev/ttyUSB1`).

## Results

| Variant | Result | Notes |
| ------- | ------ | ----- |
| `blink_tmr` | **verified** — LED1 blinks at ~1 Hz | timer IRQ path works; CPU idle between interrupts |
| `blink_busy` | **verified** — LED1 blinks at ~1 Hz | `DELAY_ITERATIONS = 10000000` ≈ 500 ms at the 60 MHz clock |

Both worked on the first flash. The ~1 Hz observation confirms the whole
timer chain: 60 MHz SYSCLK, `PERIOD = 29999999` → 2 Hz period interrupt,
`CTRL.MODE=1`, IRQ 17, and the `IC` clear in the handler.

## Findings

- **TMR1 is the right unit**: TMR0 is Embox's `jiffies` source. TMR1 at
  `0x51009000` is free; enabling it (RCU clock + reset release + SIU
  `TMREN`) does not disturb the system clock tick.
- **The timer interrupt is safe** on this board, unlike the GPIO
  interrupt: it shares the PLIC path with the clock tick (source 16 for
  TMR0, source 17 for TMR1) and there is no latch quirk.
- **Handler return type**: Embox `irq_attach()` expects an
  `irq_return_t (*)(unsigned int, void *)` handler returning
  `IRQ_HANDLED` — an `int`-returning handler fails to compile
  (`-Wincompatible-pointer-types`). Fixed in `blink_tmr.c`.
- The busy-wait `DELAY_ITERATIONS` guess (10 000 000 `nop`s) happened to
  land near 500 ms on this part; it remains a rough, CPU-bound timing
  method and will drift with clock/compiler changes, unlike the timer.

## Environment notes

- `.tmp/embox` `conf/mods.conf` had a stale live
  `gpio_button(pin_port=1, pin_num=3)` include (which breaks boot) and
  had lost the LED-counter includes; it was trimmed to just
  `blink_busy` + `blink_tmr` for this build.
- Board rebooted cleanly after the tests; shell responsive.
