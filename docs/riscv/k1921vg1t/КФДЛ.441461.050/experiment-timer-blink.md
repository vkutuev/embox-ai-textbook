# Experiment: timer blink (2026-10-13)

Hardware verification of the
[timer blink example](example-timer-blink.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 UART at `/dev/ttyUSB1`).

## Results

| Variant | Result |
| ------- | ------ |
| `blink_tmr` | **verified** — 10 blinks ≈ 10 s (exactly 1 Hz) |
| `blink_busy` | **verified** — 10 blinks ≈ 10 s (self-calibrated 1 Hz) |

## Key finding: the clock is ~189 MHz, not 60 MHz

The initial build assumed the 60 MHz system clock documented earlier.
The blink came out ~3× too fast, so the clock was re-measured:

- `mcycle` CSR advanced ~450M per ~2.4 s → **~187–190 MHz**.
- TMR1 `COUNT` advanced ~451M per ~2.4 s → **~188 MHz** (same clock).
- `blink_tmr` with `PERIOD = 189 MHz / 2 − 1` gave exactly 1 Hz, and
  `blink_busy` calibrated against `usleep()` gave exactly 1 Hz.

So the actual core/timer clock is **~189 MHz = 27 MHz HSE × 7**. The
RCU driver's PLL configuration (NDIV=100, RDIV=2, DIV1A=4, DIV1B=2) is
*intended* for 60 MHz per its own comment (`//FOUT = 60 000 000 Hz from
27 MHz HSE`), so the measured value does not match the driver's intent
— an open discrepancy worth reporting to Embox / investigating in the
PLL divider semantics. The earlier "60 MHz" note in this repo was wrong
and has been corrected.

## Other findings

- **`TMR_CTRL.MODE=0` is "Timer Stopped"**, not free-running. A
  calibration that set `MODE=0` saw the counter frozen at 0. Setting
  `MODE=1` ("count up") with a large `PERIOD` and the `CLR` bit resets
  and starts the counter. This is why the first self-calibration attempt
  measured `delta = 0`.
- **`irq_attach()` re-run leaks**: after Ctrl+C, the IRQ stays in the
  kernel IRQ table; a second `blink_tmr` run failed with "cannot attach
  TMR1 IRQ". Fixed by `irq_detach(TMR1_IRQ, NULL)` before attaching
  (same pattern as the LED/button IRQ example).
- **Handler return type**: Embox `irq_attach()` wants an
  `irq_return_t`-returning handler (`IRQ_HANDLED`); an `int` handler
  fails to compile.
- **Self-calibration approach** (final): `blink_busy` measures TMR1
  `COUNT` against `usleep(200000)` to get ticks/ms, then measures the
  delay loop to get ticks/iteration, and computes the iteration count
  for 500 ms — no hard-coded clock. Robust to the unknown PLL state.

## Environment notes

- `.tmp/embox` `conf/mods.conf` had a stale live `gpio_button` include
  (breaks boot); it was trimmed to just `blink_busy` + `blink_tmr`.
- Board rebooted cleanly after the tests; shell responsive.
