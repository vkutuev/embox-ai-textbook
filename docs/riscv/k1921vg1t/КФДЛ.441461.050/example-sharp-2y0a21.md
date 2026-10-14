# Example: Sharp GP2Y0A21 IR distance sensor

Measure distance with a Sharp GP2Y0A21YK0F analog IR sensor through the
12-bit ADC (channel 8, PA8).

> **Status: verified on hardware (2026-10-14)** — `sharp` reads values
> that track distance and the reported distance matches a ruler closely.
> See the [experiment journal](experiment-sharp-2y0a21.md).

## Hardware

| Sharp 2Y0A21 | Board |
| ------------ | ----- |
| VCC (red) | XP3 pin 29 (+5 V) |
| GND (black) | XP1 pin 2 |
| Vout (yellow) | XP1 pin 27 (PA8, ADC channel 8) |

- The sensor is a 5 V device; power it from 5 V.
- Output ~3.1 V at 10 cm → ~0.4 V at 80 cm, within the 3.3 V ADC range.
  **Keep objects ≥ 10 cm**: closer, the output can rise toward 5 V and
  exceed the ADC input limit.

## Embox support

Embox has no ADC driver, so the command programs the ADC directly — the
same registers as the
[temperature-sensor example](example-adc-tsensor.md), but for block 0
(`ACTL[0]`, channels 0-15) and channel 8:

1. Make PA8 a hi-Z input (clear `OUTEN`/`ALTFUNC`).
2. RCU `ADCCFG` (HSE) + `CGCFGAHB`/`RSTDISAHB`.
3. `ACTL[0]`: LDO, 12-bit, enable, wait `ADCRDY`.
4. Sequencer 0: 3 requests of channel 8, `CHDELAY[8]`.
5. Per sample: pulse `GSYNC` (keeping `SYNC0`), read 3 values from
   `SFIFO`, average.

Distance: `d = 30.86 / (V − 0.014)` (V in volts) — a reciprocal fit to
the 10 cm / 80 cm datasheet points; treat as approximate.

## Build and run

1. Copy `examples/niet-dev-k1921vg1t/sharp_2y0a21/` into
   `<embox>/src/cmds/example/` and add the `include` from
   `mods.conf.snippet`.
2. Build, flash, reset.
3. Aim at a flat object and run:

   ```
   root@embox:/#sharp 5
   sharp: PA8 (ADC ch8), 5 samples
   [1] raw=771  V=621 mV  d=50 cm
   ...
   ```

## Verification checklist (on hardware)

1. `sharp` prints a voltage that changes with distance. ✔ (2026-10-14)
2. The reported distance matches a ruler (within a few cm). ✔

## Notes

- Recalibrate `30860` / `14` (in mV) if a specific unit reads off.
- External ADC channels use the same sequencer mechanism; the temp sensor
  (channel 63) is read the same way in the `tsensor` example.

## Sources

- ADC chapter: [adc.md](../adc.md).
- Sharp GP2Y0A21YK0F data sheet.
