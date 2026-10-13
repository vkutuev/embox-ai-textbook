# Example: timer blink

Blink LED1 — first with a busy-wait loop, then with a hardware timer
interrupt. The contrast shows why real-time code uses timers instead
of counting CPU cycles.

> **Status: verified on hardware (2026-10-13)** — both variants blink
> LED1 at ~1 Hz; see the
> [experiment journal](experiment-timer-blink.md).

## Hardware

| Item | Value |
| ---- | ----- |
| LED1 | GPIOC pin 12, active-high |
| Timer | TMR1 at `5100_9000h` (TMR0 is Embox's clock tick) |
| Timer clock | SYSCLK = 60 MHz (actual) |
| Period IRQ | PLIC source 17 (TMR0–TMR15 = sources 16–31) |

The timer must be enabled before counting:

- `RCU.CGCFGAPB1.TMR1EN` (bit 17) — clock gate
- `RCU.RSTDISAPB1.TMR1EN` (bit 17) — release reset
- `SIU.TMREN.TMR1EN` (bit 1) — start counting

With `CTRL.MODE=1` the counter runs 0..`PERIOD` and sets the period
interrupt flag at the wrap; the handler writes 1 to `IC` to clear it.

## Variants

Both are in
[`examples/niet-dev-k1921vg1t/timer_blink/`](../../../../examples/niet-dev-k1921vg1t/timer_blink/):

| Variant | Command | Period source | CPU cost |
| ------- | ------- | ------------- | -------- |
| Busy-wait | `blink_busy` | calibrated delay loop | 100% while waiting |
| Timer | `blink_tmr` | TMR1 period interrupt at 2 Hz | idle between interrupts |

- **Busy-wait** is simple but burns the CPU and is only as accurate as
  the loop's calibration (and it drifts with the compiler/clock).
- **Timer** gives an exact, hardware-driven period and leaves the CPU
  free. It uses the same interrupt path as Embox's own clock tick, so
  it is safe on this board (unlike GPIO interrupts — see the LED/button
  example's experiment journal).

For `blink_tmr`, `PERIOD = 60 MHz / 2 − 1 = 29999999`: the interrupt
fires at 2 Hz, and each interrupt toggles LED1, giving one full on/off
cycle per second (1 Hz).

## Build and run

1. Copy the example directory into an Embox checkout:

   ```console
   $ cp -r examples/niet-dev-k1921vg1t/timer_blink/ <embox>/src/cmds/example/
   ```

2. Add the include for one variant to
   `platform/niiet/templates/k1921vg1t_dev/mods.conf` (see
   `mods.conf.snippet` in the example directory), build, flash, reset.

3. Run it:

   ```
   root@embox:/#blink_tmr
   blink_tmr: LED1 blinking at 1 Hz (Ctrl+C to stop)
   ```

## Verification checklist (on hardware)

1. `blink_tmr`: LED1 blinks steadily; time 20 blinks with a stopwatch —
   expect ~20 s (1 Hz).
2. `blink_busy`: LED1 blinks at roughly the same rate; note the drift
   and adjust `DELAY_ITERATIONS` if far off.
3. Confirm the system stays responsive during `blink_tmr` (the shell is
   not blocked; the timer interrupt is shared with jiffies, which still
   ticks).
4. Optional: with GDB, read TMR1 `PERIOD`/`COUNT`/`IM`/`IC` to confirm
   the configuration.
5. Record results in the experiment journal.

Checked on 2026-10-13: `blink_tmr` and `blink_busy` both blink LED1 at
~1 Hz; the shell stays responsive during `blink_tmr`. See
[experiment-timer-blink.md](experiment-timer-blink.md).

## Sources

- TMR peripheral: UM chapter 16, appendix A.10 (see
  [timer.md](../timer.md)).
- SDK reference (`run_leds` example) for the TMR enable sequence.
