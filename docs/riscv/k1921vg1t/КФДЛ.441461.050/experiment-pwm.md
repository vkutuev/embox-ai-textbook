# Experiment: PWM output (2026-10-13)

Hardware verification of the [PWM example](example-pwm.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console, logic analyzer on
XP1 pin 19).

## Result

**Verified.** `pwm_demo 1000 50` produced 1.0 kHz at 50% duty (500 µs
high); `pwm_demo 1000 25` produced 1.0 kHz at 25% duty (250 µs high);
`pwm_demo sweep 1000` swept the duty 0%..100%..0%.

## Findings

- The `k1921vg1t_dev` template already enables `niiet_pwm1` (TMR1 in PWM
  mode) and the stock `pwm` command. PWM1 OUT0-OUT3 map to PA4-PA7
  (alt function 5); PA4 is XP1 pin 19.
- PWM1 and SPI1 share pins PA4-PA7. In the stock template PWM1 wins
  (verified via GDB: `GPIOA.ALTFUNCNUM0` nibbles 4-7 = 5, i.e. alt 5).
- The stock `pwm` command's `-d` passes the channel number rather than
  `1 << chan` to `pwm_enable()`, so `pwm -d 1 1 X` actually enables
  channel 0. The `pwm_demo` command uses `pwm_enable(dev, 1 << chan)`
  correctly.
- Register-level check after `pwm_demo 1000 25`: `PERIOD = 0x249EF`
  (149999), `CAPCOM[0].VAL = 0x927B` (37499), `CTRL = 0x1E10`
  (MODE=up, OUTEN0-3). Correct.

## Clock finding

`pwm_demo 1000` produced **exactly 1.0 kHz** and a 50% duty gave a
**500 µs** high time. Both are consistent only with a **150 MHz** timer
clock, matching the board config `SYSTEM_FREQ = 150 MHz`.

This conflicts with the [timer blink experiment](experiment-timer-blink.md),
which measured ~189 MHz (mcycle and TMR1 COUNT over a wall-clock
interval) and used `PERIOD = 189 MHz / 2 − 1` to get a 1 Hz blink. Both
observations were made on hardware at different times and different
builds; the discrepancy is unresolved and should be re-measured in one
controlled pass (CPU `mcycle` vs TMR `COUNT` over the same interval)
before either number is treated as final.

## Environment notes

- A GDB attach halts the target; an SRST pulse recovers it.
- The CH340 console corrupts long output; short command output is
  reliable.
