# Internal temperature sensor example for NIIET-DEV-K1921VG1T

Read the VG1T's internal temperature sensor through the 12-bit ADC
(channel 63, ADC block 3) and print the temperature in degrees Celsius.
No wiring needed.

## Install into Embox

1. Copy `adc_tsensor/` into `<embox>/src/cmds/example/`.
2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the line
   from `mods.conf.snippet`.
3. Build, flash, reset.

## Run and verify

```
root@embox:/#tsensor 5
tsensor: ADC ch63 (internal temp sensor), 5 samples
[1] raw=931  V=750 mV  T=24.7 C
...
```

The temperature should read near room temperature (~20-30 °C). Touch or
blow on the MCU to see the reading change.

## How it works

Embox has no ADC driver, so the command programs the ADC directly:
enable the RCU ADC clock (HSE), configure `ACTL[3]` (LDO, 12-bit,
enable), start it, set up sequencer 0 for channel 63 (3 samples), enable
the sensor via SIU `TSENSCTRL`, then trigger conversions and read the
FIFO. Temperature is `574.713 · (0.793 − V)` for the scaled output,
computed in fixed point (V in volts from `raw/4096 · 3.3`).

The absolute temperature assumes `AVCC = 3.3 V` and the on-chip sensor's
calibration; expect it to be approximate.
