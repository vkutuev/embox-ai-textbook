# Timers (TMR)

Sixteen general-purpose 32-bit timers, TMR0–TMR15. Each has a
free-running/period counter, a clock divider, a period (compare)
register, interrupt control, and four capture/compare (CAPCOM) blocks.

| Timer | Base | Timer | Base |
| ----- | ---- | ----- | ---- |
| TMR0 | `5100_8000h` | TMR8 | `5101_0000h` |
| TMR1 | `5100_9000h` | TMR9 | `5101_1000h` |
| TMR2 | `5100_A000h` | TMR10 | `5101_2000h` |
| TMR3 | `5100_B000h` | TMR11 | `5101_3000h` |
| TMR4 | `5100_C000h` | TMR12 | `5101_4000h` |
| TMR5 | `5100_D000h` | TMR13 | `5101_5000h` |
| TMR6 | `5100_E000h` | TMR14 | `5101_6000h` |
| TMR7 | `5100_F000h` | TMR15 | `5101_7000h` |

## Clock and enable

- Timer clock = the system clock (SYSCLK) selected by `CLKSEL` (bit 8),
  divided by `CLKDIV`. On the dev board the timer clock was measured as
  ~189 MHz by the timer-blink example (27 MHz HSE × 7), but as 150 MHz by
  the [PWM example](pwm.md) — an unresolved discrepancy; see the
  [PWM experiment](КФДЛ.441461.050/experiment-pwm.md). The RCU driver's
  PLL config is intended for 60 MHz.
- Enable the timer clock: `RCU.CGCFGAPB1.TMRnEN` and release the reset:
  `RCU.RSTDISAPB1.TMRnEN` (bit 16 + n).
- Start counting: `SIU.TMREN.TMRnEN` (bit n).

## Register map (per timer)

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CTRL | control: `CLR` (bit 2) resets the counter; `MODE` (bits 4–5) selects the count mode; `CLKSEL` (bit 8) selects the clock source |
| +04h | COUNT | current counter value |
| +08h | CLKDIV | clock divider |
| +0Ch | PERIOD | period (compare) value |
| +10h | IM | interrupt mask; bit 0 = period interrupt |
| +14h | RIS | raw interrupt status |
| +18h | MIS | masked interrupt status |
| +1Ch | IC | interrupt clear (write 1 to clear) |
| +20h | DMA_TXIM | DMA TX request mask |
| +24h | DMA_RXIM | DMA RX request mask |
| +28h | EXTEVT_IM | external event mask |
| +54h | CAPCOM[0..3] | capture/compare blocks |

Count modes (`CTRL.MODE`):

- `0` — free-running up-counter (wraps at 0xFFFF_FFFF).
- `1` — up-counter from 0 to `PERIOD`; on wrap the period interrupt
  flag is set and the counter restarts.

## Interrupts

- General (period) interrupts: PLIC sources 16–31 = TMR0–TMR15.
- Capture/compare interrupts: PLIC sources 88–119 (TMR0_CC0–TMR3_CC7).
- Period interrupts (separate): PLIC sources 152–155 (TMR0–TMR3_PERIOD).

The period interrupt fires when the counter wraps at `PERIOD` (with
`MODE=1`); the handler must clear it by writing 1 to `IC`.

## Usage on the dev board

TMR0 is Embox's clock-tick source (`jiffies`), so examples use TMR1.
See the [timer blink example](КФДЛ.441461.050/example-timer-blink.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 16 and appendix A.10
