# Timer blink example for NIIET-DEV-K1921VG1T

Two ways to blink LED1 — the contrast between busy-wait timing and a
hardware timer interrupt.

| Variant | Command | How the period is made |
| ------- | ------- | ---------------------- |
| Busy-wait | `blink_busy` | CPU spins in a calibrated delay loop |
| Timer | `blink_tmr` | TMR1 period interrupt at 2 Hz toggles LED1 |

## Hardware

- **TMR1** at `0x51009000` (TMR0 is Embox's clock-tick source — do not
  touch it). Timer clock = 60 MHz system clock.
- Enable the timer: `RCU.CGCFGAPB1.TMR1EN` and `RCU.RSTDISAPB1.TMR1EN`
  (bit 17), then `SIU.TMREN.TMR1EN` (bit 1).
- Registers: `CTRL` (+0x00, `MODE` bits 4–5), `PERIOD` (+0x0C), `IM`
  (+0x10, bit 0 = period IRQ), `IC` (+0x1C, write 1 to clear).
- **PLIC IRQ: TMR1 = source 17** (TMR0–TMR15 = sources 16–31).

## Install into Embox

1. Copy this directory into an Embox checkout:

   ```console
   $ cp -r timer_blink/ <embox>/src/cmds/example/
   ```

2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the
   include line for the variant you want (see `mods.conf.snippet`).

3. Build, flash, run (`blink_busy` or `blink_tmr`).

## Expected behavior

- `blink_tmr`: LED1 blinks at exactly 1 Hz (one on/off cycle per
  second); the CPU stays idle between interrupts.
- `blink_busy`: LED1 blinks at approximately the same rate; the CPU is
  busy the whole time, and the rate drifts with the clock and compiler
  optimizations (`DELAY_ITERATIONS` is a starting guess).

## Notes

- `blink_tmr` uses `irq_attach(17, ...)` — the same interrupt path as
  Embox's own clock tick, so it is safe on this board (unlike the GPIO
  interrupt path, which needs a mask-and-re-arm discipline; see the
  LED/button example).
- The timer's `CTRL.MODE=1` counts 0..`PERIOD` and raises the period
  interrupt at the wrap.
