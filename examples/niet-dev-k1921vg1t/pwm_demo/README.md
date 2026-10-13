# PWM example for NIIET-DEV-K1921VG1T

Generate a PWM waveform using the niiet PWM driver (a TMR unit in PWM
mode) and observe it on a logic analyzer.

## Hardware

| Signal | Board |
| ------ | ----- |
| PWM1 OUT0 (PA4) | XP1 pin 19 |
| GND | XP1 pin 2 (or any GND) |

Channels OUT0–OUT3 are PA4–PA7 (XP1 pins 19/21/23/25), alt function 5.

## Install into Embox

The `k1921vg1t_dev` template already enables the PWM driver
(`niiet_pwm1` on TMR1) and the stock `pwm` command. To add this example:

1. Copy `pwm_demo/` into `src/cmds/example/`.
2. Add the `include` from `mods.conf.snippet`.
3. (Recommended) Disable SPI1 (`pl022_spi1` + `spi1_dev0`), which shares
   pins PA4–PA7.
4. Build, flash, reset.

## Run and verify

```
root@embox:/#pwm_demo 1000 50
pwm_demo: PWM1 chan0 freq=1000 Hz duty=50%

root@embox:/#pwm_demo sweep 1000     # duty 0%..100%..0%
```

On the logic analyzer (CH on XP1 pin 19, GND common) you see a square
wave. Note the frequency: requesting 1000 Hz yields ~1260 Hz because the
driver assumes `SYSTEM_FREQ = 150 MHz` while the real TMR clock is
~189 MHz. The duty *percentage* is preserved.

The stock `pwm` command is an alternative (`pwm -f 1 1000`,
`pwm -d 1 0 500000`), but note its `-d` passes the channel number rather
than a bitmask to `pwm_enable()`.
