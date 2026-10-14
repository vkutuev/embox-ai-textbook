# Experiment: Sharp GP2Y0A21 distance sensor (2026-10-14)

Hardware verification of the
[Sharp distance-sensor example](example-sharp-2y0a21.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console).

## Result

**Verified.** `sharp` reads the sensor's analog output on PA8 (ADC
channel 8, block 0); the voltage tracks distance and the reported
distance matches a ruler closely (recalibration not needed).

## Wiring

| Sharp 2Y0A21 | Board |
| ------------ | ----- |
| VCC (red) | XP3 pin 29 (+5 V) |
| GND (black) | XP1 pin 2 |
| Vout (yellow) | XP1 pin 27 (PA8, ADC channel 8) |

## Findings

- Reuses the register-level ADC setup from the
  [temperature-sensor experiment](experiment-adc-tsensor.md), with two
  changes: block 0 (`ACTL[0]`, channels 0-15) instead of block 3, and
  channel 8 instead of 63.
- The analog pin must be made a hi-Z input — clear `GPIOA.OUTENCLR` and
  `GPIOA.ALTFUNCCLR` for PA8 — so the ADC can sample it (the SDK `adc`
  example does the same for the whole port).
- Distance is a reciprocal fit `d = 30.86 / (V − 0.014)` (V in volts),
  computed in fixed point as `d = 30860 / (V_mV − 14)`. It matches the
  datasheet's 10 cm / 80 cm points and reads correctly on this unit.
- The sensor is a 5 V device; its output stays within the 3.3 V ADC
  range for objects ≥ 10 cm. Closer than 10 cm the output can rise
  toward 5 V, so a series resistor is recommended for close-range use.

## Environment notes

- The CH340 console corrupts long output; short command output is
  reliable.
