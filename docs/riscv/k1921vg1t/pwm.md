# PWM

The VG1T generates PWM with its timer units (TMR0-TMR15) running in PWM
mode. Each timer has up to four capture/compare (CAPCOM) blocks, so one
timer produces up to four PWM outputs (OUT0-OUT3).

## Timer / PWM register map

Base addresses: `TMRn` at `5100_8000h + n * 1000h` (TMR0 = `5100_8000h`,
TMR1 = `5100_9000h`, ...).

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CTRL | control: `MODE` (bits 4-5: 1 = up, 2 = continuous, 3 = up/down), `CLR` (bit 2), `CLKSEL` (bit 8), `OUTEN` (bits 9-12), `DIV` (bits 6-7) |
| +04h | COUNT | current counter value |
| +08h | CLKDIV | clock divider (reset 0) |
| +0Ch | PERIOD | period in "up" / "up-down" modes |
| +10h | IM / +14h RIS / +18h MIS / +1Ch IC | interrupt mask/status |
| +100h + 10h·n | CAPCOM[n].CTRL | `OUTMODE` (bits 5-7): 3 = set-on-match, 7 = reset-on-match |
| +104h + 10h·n | CAPCOM[n].VAL | compare value (duty) for OUTn |

## Clock

The timer counts on SYSCLK (CLKSEL = 0) or an external signal on TMR_IO
(CLKSEL = 1), optionally through the CLKDIV divider. The peripheral bus
clock (PCLK) only gates register access (table 6.4 of the user manual).

Measured on the dev board: the timer clock is **150 MHz** (matching the
board config `SYSTEM_FREQ`), so `PERIOD = 150 MHz / f − 1` yields the
requested PWM frequency exactly. (See the PWM experiment note — an
earlier timer measurement reported ~189 MHz and remains to be reconciled.)

## Embox driver

The `k1921vg1t_dev` template enables `embox.driver.pwm.niiet_pwm1` (TMR1)
and the stock `pwm` command. The board config maps PWM1 OUT0-OUT3 to
PA4-PA7 (alt function 5), exposed on XP1 pins 19/21/23/25.

The core API (`<drivers/pwm.h>`): `pwm_dev_by_id()`,
`pwm_set_frequency()` (Hz), `pwm_set_duty()` (duty in ns), and
`pwm_enable()` (a *bitmask* of channels). Note the stock `pwm` command's
`-d` passes the channel number rather than `1 << chan` to `pwm_enable()`
(a bug); the [PWM example](КФДЛ.441461.050/example-pwm.md) uses the API
directly.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 16 (TMR) and
  appendix A (register map).
