# Sharp GP2Y0A21 distance sensor example for NIIET-DEV-K1921VG1T

Measure distance with a Sharp GP2Y0A21YK0F IR sensor through the 12-bit
ADC (channel 8, PA8).

## Hardware

| Sharp 2Y0A21 | Board |
| ------------ | ----- |
| VCC (red) | XP3 pin 29 (+5 V) |
| GND (black) | XP1 pin 2 |
| Vout (yellow) | XP1 pin 27 (PA8, ADC channel 8) |

- The sensor is a 5 V device; power it from 5 V.
- Its output is ~3.1 V at 10 cm down to ~0.4 V at 80 cm, within the 3.3 V
  ADC range. **Keep objects ≥ 10 cm** — closer than that the output can
  rise toward 5 V and exceed the ADC input limit.

## Install into Embox

1. Copy `sharp_2y0a21/` into `<embox>/src/cmds/example/`.
2. Add the `include` from `mods.conf.snippet`.
3. Build, flash, reset.

## Run and verify

```
root@embox:/#sharp 5
sharp: PA8 (ADC ch8), 5 samples
[1] raw=932  V=751 mV  d=42 cm
...
```

Aim the sensor at a flat object (e.g. a book) at a known distance and
check against a ruler. Distance is `d = 30.86 / (V − 0.014)` (V in volts)
— a reciprocal fit to the 10 cm / 80 cm datasheet points, so treat it as
approximate and recalibrate `30860` / `14` (mV) for your unit.
