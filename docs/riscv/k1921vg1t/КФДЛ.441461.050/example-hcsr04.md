# Example: HC-SR04 ultrasonic distance

Measure distance with an HC-SR04 ultrasonic sensor, timing the ECHO pulse
with the TMR1 free-running counter (189 MHz, 27 MHz HSE × 7).

> **Status: not verified on this board — requires 5 V + a resistor
> divider.** The HC-SR04 does not respond when powered from 3.3 V (it is
> specified for 4.5-5.5 V); this was confirmed by probing the sensor pins
> directly with a logic analyzer. See the
> [experiment journal](experiment-hcsr04.md). The code below is complete
> and only needs the 5 V wiring.

## Hardware

| HC-SR04 | Board |
| ------- | ----- |
| VCC | XP3 pin 29 (5 V) |
| GND | XP1 pin 2 |
| TRIG | XP1 pin 9 (PD0) |
| ECHO | `10 kΩ` → junction → `20 kΩ` → GND; junction → XP1 pin 7 (PD1) |

The 10 kΩ / 20 kΩ divider drops the 5 V ECHO to ~3.3 V so it stays within
the MCU's input limit (`UIH` max `UCC1 + 0.6 V`). Any roughly 1:2 resistor
pair works (e.g. 10k/22k, 4.7k/10k).

## Build and run

1. Copy `examples/niet-dev-k1921vg1t/hcsr04/` into an Embox checkout
   (`src/cmds/example/`) and add the `include` line from
   `mods.conf.snippet` to `mods.conf`.
2. Build, flash, reset.
3. On the console:

   ```
   root@embox:/#hcsr04 5
   hcsr04: TRIG=PD0 ECHO=PD1 (5 samples)
   [1] echo=870 us, dist=15 cm
   ...
   ```

Distance is `echo_us / 58` (speed of sound ~343 m/s, round trip). A logic
analyzer on TRIG (20 µs pulse) and ECHO (width proportional to distance)
shows the whole transaction.

## Why not 3.3 V

The sensor is a 5 V device. At 3.3 V its transducer drive is too weak to
produce a detectable echo, and its ECHO output never toggles (verified on
hardware). Powering it from 5 V requires the divider on ECHO, since the
MCU pins are not 5 V-tolerant.
