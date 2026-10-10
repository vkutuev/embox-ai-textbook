# Clocks and Reset (RCU)

The RCU block (base `5000_4000h`) manages clock generation and reset.

## Clock sources

| Signal | Source | Frequency |
| ------ | ------ | --------- |
| LSICLK | internal RC of the RTC block | 32.768 kHz |
| LSECLK | external crystal at XI_RTC/XO_RTC | 32.768 kHz |
| EXTCLK | external clock at GPIOA3 or GPIOD6 | — |
| HSICLK | internal RC oscillator | ~4 MHz |
| HSECLK | external crystal at XI_OSC/XO_OSC | 2-27 MHz |
| PLL0CLK | PLL0 output | up to 204 MHz |
| PLL1CLK | PLL1 output (SDRAM, I2S, USB, Ethernet, CANFD) | — |
| PLL2CLK | PLL2 output (SDRAM, I2S, USB, Ethernet, CANFD) | — |
| CLKOUT | output clock, source selected by CLKSEL in CLKOUTCFG | — |

`SYSCLK`, the system clock that drives the CPU, is selected by the `SRC`
field of the `SYSCLKCFG` register (offset +20h):

- `00` — HSICLK (default after reset)
- `01` — HSECLK
- `10` — PLL0CLK
- `11` — EXTCLK

Clock gating to peripherals is controlled by `CGCFGAHB` and `CGCFGAPB`
registers.

## PLL configuration

PLL output frequency (integer mode, pre-divider bypassed):

```
Fout = Fvco / ((DIV1A + 1) * (DIV1B + 1))
Fvco = Fref * N / (RDIV + 1)
```

where `Fref` is the reference clock, `N` is the feedback divider (1-256),
`RDIV` the reference divider (1-64). With the pre-divider enabled
(`VCOMODE[3] = 1`), `PR` (1, 2, 4) multiplies `Fvco`. Fractional mode:
`N = NDIV + FRAC/MOD` (MOD from 2 to 2^24-1).

Limits:

- `Fref`: 10-100 MHz
- PFD input: 10 MHz to Fvco/16 (integer), 10 MHz to Fvco/20 (fractional)
- N: 1-160 (integer), 20-160 (fractional)
- `Fvco`: 400-1000 MHz
- `Fout`: 200 kHz to 204 MHz

Configuration sequence (must be done before switching SYSCLK to the PLL):

1. Clear `BYPASS`, clear `PD` (wake up) in `PLLCFG`.
2. Set `PREDIV`, `DIV1A`, `DIV1B`, `RDIV`, `NDIV` in `PLLDIV`; set `RNCLR`
   and `NNCLR`. In fractional mode set `ST` in `PLLCFG` and write `FRAC`
   (`PLLFRAC`) and `MOD` (`PLLMOD`).
3. Select the reference source with `CLKSEL`, set PFD/charge-pump to
   working mode (11b), select VCO sub-band (recommended 11b), set `FOUTEN`.
4. Wait for `LOCK` in `PLLSTAT`.

## CPE pin note

The SDK header comments mention a `CPE` pad that inverts the meaning of
the HSI/HSE clock-source selects. The user manual does not document CPE
at all (no pin in the package table), and on real hardware the SIU
`CPESTAT` register (offset +08h) reads 0, so the selects are not
inverted on this chip: CLKSEL=1 means HSE (27 MHz on the dev board).
Treat the SDK header comment as generic; do not rely on it for the
K1921VG1T.

## HSI RC oscillator

Internal RC oscillator, nominal 4 MHz. Frequency is tunable via the `TUNE`
field of the SIU `RCCTRL` register (3.894 MHz at TUNE=0 down to 2.553 MHz
at TUNE=255; default TUNE=66).

## HSE oscillator

External crystal 2-27 MHz on XI_OSC/XO_OSC. Recommended load capacitors:
50 pF at 2 MHz, 16 pF at 27 MHz. Startup time up to 30 ms.

## Reset signals

- Power-on/brown-out reset (POR/BOR) supervisors included.
- External reset: `RST#` low for at least 10 µs.
- Per-peripheral reset bits live in RCU registers (e.g. `UARTCFGi.RSTDIS`
  releases UART i from reset).
- A clock-monitoring subsystem detects loss of HSI/HSE/EXTCLK/PLL0 clocks.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 6
