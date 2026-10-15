# Example: PWM output

Generate a PWM waveform using the niiet PWM driver (TMR1 in PWM mode) and
observe it on a logic analyzer.

> **Status: verified on hardware (2026-10-13)** — `pwm_demo 1000 50`
> produced 1.0 kHz at 50% duty (500 µs high), and the duty sweep worked.
> See the [experiment journal](experiment-pwm.md).

## Hardware

| Signal | Board |
| ------ | ----- |
| PWM1 OUT0 (PA4) | XP1 pin 19 |
| GND | XP1 pin 2 |

Channels OUT0-OUT3 are PA4-PA7 (XP1 pins 19/21/23/25), alt function 5.

## Embox support

The template already enables the driver and the stock command:

```
include embox.driver.pwm.niiet_pwm_drv(vg1t_version=1)
include embox.driver.pwm.niiet_pwm1(base_addr=0x51009000)
include embox.cmd.hw.pwm
```

PWM1 (TMR1) shares pins PA4-PA7 with SPI1; in the stock template PWM1
wins those pins, but disabling SPI1 (`pl022_spi1` + `spi1_dev0`) is
recommended. The example adds a small command that uses the PWM core API
directly:

```
include embox.cmd.example.pwm_demo.pwm_demo
```

## Build and run

1. Write the command module in `<embox>/src/cmds/example/pwm_demo/` (see
   [Commands and drivers](../../../embox/commands-drivers.md)) and add the
   `include embox.cmd.example.pwm_demo.pwm_demo` line shown above.
2. Build, flash, reset.
3. On the console:

   ```
   root@embox:/#pwm_demo 1000 50
   pwm_demo: PWM1 chan0 freq=1000 Hz duty=50%
   root@embox:/#pwm_demo sweep 1000
   ```

On the logic analyzer (CH on XP1 pin 19, GND common) you see a square
wave at the requested frequency with the requested duty percentage.

## Verification checklist (on hardware)

1. `pwm_demo 1000 50` → 1.0 kHz, 50% duty. ✔ (2026-10-13)
2. `pwm_demo 1000 25` → 1.0 kHz, 25% duty. ✔
3. `pwm_demo sweep 1000` → duty ramps 0%..100%..0%. ✔
4. Frequency matches the request exactly, confirming the 150 MHz timer
   clock.

## Sources

- PWM chapter: [pwm.md](../pwm.md).
- Embox PWM core API: `src/drivers/pwm/pwm_dev.c`,
  `src/drivers/pwm/niiet/`.
