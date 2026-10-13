# HC-SR04 ultrasonic example for NIIET-DEV-K1921VG1T

Measure distance with an HC-SR04 ultrasonic sensor, timed with the TMR1
free-running counter (189 MHz).

> **Requires 5 V + a resistor divider.** The HC-SR04 does not respond at
> 3.3 V (out of spec, 4.5-5.5 V) — verified on hardware. Power it from
> 5 V and divide ECHO down to 3.3 V.

## Hardware

| HC-SR04 | Board |
| ------- | ----- |
| VCC | XP3 pin 29 (+5 V) |
| GND | XP1 pin 2 (GND) |
| TRIG | XP1 pin 9 (PD0) |
| ECHO | `10 kΩ` → junction → `20 kΩ` → GND; junction → XP1 pin 7 (PD1) |

- The 10 kΩ / 20 kΩ divider drops the 5 V ECHO to ~3.3 V, keeping it
  within the MCU pin limit (`UIH` max `UCC1 + 0.6 V`). Any ~1:2 resistor
  pair works (e.g. 10k/22k, 4.7k/10k).
- Do **not** connect a 5 V ECHO directly to the MCU pin.

## Install into Embox

1. Copy this directory into an Embox checkout:

   ```console
   $ cp -r hcsr04/ <embox>/src/cmds/example/
   ```

2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the line
   from `mods.conf.snippet`.

3. Build, flash, reset.

## Run and verify

```
root@embox:/#hcsr04
hcsr04: TRIG=PD0 ECHO=PD1 (1 sample)
[1] echo=870 us, dist=15 cm

root@embox:/#hcsr04 5
[1] echo=872 us, dist=15 cm
[2] echo=870 us, dist=15 cm
...
```

Hold a flat object (or your hand) at a known distance and check the
reported value against a ruler. A logic analyzer on TRIG (20 µs pulse)
and ECHO (width proportional to distance) shows the whole transaction.
