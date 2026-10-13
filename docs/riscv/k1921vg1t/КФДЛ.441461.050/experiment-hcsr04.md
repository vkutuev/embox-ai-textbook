# Experiment: HC-SR04 ultrasonic distance (2026-10-13)

Attempt to drive an HC-SR04 ultrasonic sensor from the board's **3.3 V**
rail, with ECHO read directly on a GPIO pin (the 3.3 V supply was chosen
so ECHO stays within the MCU's input limits, avoiding a level shifter).

## Result

**Not usable at 3.3 V.** The HC-SR04 is specified for 4.5-5.5 V; powered
from 3.3 V it does not produce an echo. The example code is correct and
is kept for a 5 V + divider setup (no code change needed).

## Wiring attempted

| HC-SR04 | Board |
| ------- | ----- |
| VCC | XP3 pin 19 (3.3 V) |
| GND | XP1 pin 2 |
| TRIG | XP1 pin 9 (PD0) |
| ECHO | XP1 pin 7 (PD1) |

## Diagnostic journey

1. Console reported `no echo (ECHO stayed low)` — ECHO never rose.
2. Logic analyzer on the **board** pins: TRIG (D0) pulses, ECHO (D1) flat
   — but this only shows the MCU side.
3. Logic analyzer on the **sensor** pins (via breadboard): VCC = 3.3 V
   (powered), TRIG pulses (trigger received), ECHO flat high — the sensor
   fires but never echoes.

## Findings

- The HC-SR04 does not work at 3.3 V: powered and triggered, its ECHO
  output never toggles. This is out-of-spec operation (datasheet:
  4.5-5.5 V) and a confirmed failure mode on this module.
- The MCU side is correct: PD0 drives a ~20 µs TRIG pulse, PD1 reads the
  ECHO level. Both verified by direct pin probing.
- While the sensor was not driving ECHO, the MCU's floating input read
  low in one setup and high in another — a floating input should not be
  mistaken for a driven signal. Always probe the device under test, not
  just the MCU pins.
- To use the sensor: power from 5 V and divide ECHO with a 10 kΩ / 20 kΩ
  divider (any ~1:2 ratio) so the 5 V ECHO drops to ~3.3 V. The `hcsr04`
  command needs no changes.
