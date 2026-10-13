# Experiment: PCA9685 PWM driver over I2C (2026-10-13)

Hardware verification of the
[PCA9685 I2C example](example-i2c-pca9685.md) on the КФДЛ.441461.050 board
(J-Link V9 + CH340 console at `/dev/ttyUSB0`, NXP PCA9685 on I2C0).

## Result

**Verified**: the PCA9685 responds on I2C0 at `0x40`, the `pca9685`
command reads/writes its registers, and the generated PWM was observed in
PulseView on a logic analyzer.

## Wiring (verified)

| Board | Signal | PCA9685 module |
| ----- | ------ | -------------- |
| XP1 pin 5 | PD2 = I2C0_SDA | SDA |
| XP1 pin 3 | PD3 = I2C0_SCL | SCL |
| XP1 pin 2 (any GND) | GND | GND |
| XP3 pin 19 | +3.3 V | VCC (logic) and V+ (outputs) |

Device at the default address `0x40` (A0-A5 low).

## Findings

- The I2C0 bus is already enabled by the `k1921vg1t_dev` template
  (`embox.driver.i2c.niiet.vg015_i2c0`), which configures PD2/PD3 as
  alt function 2, open-drain with internal pull-up, and enables
  `CLK_I2C0`. Confirmed via GDB after boot: `GPIOD.ALTFUNCSET = 0x0C`,
  `ALTFUNCNUM0 = 0x2200` (alt 2), `I2C0.CTL1 = 0xFB` (ENABLE,
  SCLFRQ = 125), bus idle (no "bus busy" warning).
- `pca9685 info` before `init` reads the power-on defaults:
  `MODE1 = 0x11` (SLEEP + ALLCALL), `MODE2 = 0x10`, `PRE_SCALE = 30`
  (~196 Hz).
- `pca9685 init 1000` sets `PRE_SCALE = 5` -> calculated 1017 Hz,
  `MODE1 = 0x20` (awake, auto-increment), `MODE2 = 0x04` (totem-pole).
- `pca9685 set 0 2048` writes `LED0_OFF = 0x0800` (verified by reading
  register `0x09` back as `0x08`); `set 0 1024` -> `0x04`. The duty
  write works.
- **Measured PWM frequency is 1.1 kHz**, not the calculated 1017 Hz.
  That means the PCA9685's internal oscillator runs at ~27 MHz, not the
  nominal 25 MHz (+8%, inside the chip's ±10% spec). The PRE_SCALE
  quantization itself contributes only +1.7% (1017 vs 1000). Conclusion:
  PWM frequency from the PCA9685 is only as accurate as its internal RC
  oscillator; an exact frequency needs an external clock on `EXTCLK`.
- `i2cdetect -r` (read-byte scan) trips the controller into HS mode on
  the reserved addresses (`0x04`-`0x07` become "master codes"); the
  `vg015_i2c` RX path logs `RX: expected MRADPA, got 0x21` (HMTMCOK)
  instead of treating it as no-device. Cosmetic, not a transfer failure.
- Because `MODE1` enables ALLCALL by default, the chip also answers its
  ALLCALL address `0x70` (seen as `70` in the read scan).

## Not verified

- Actual SCL frequency. The driver hardcodes `I2C_SYSTEM_FREQ = 50 MHz`
  for the divider; SCL was not probed, so the `CLK_I2C0` value is
  unconfirmed. (The bus works regardless, which is what matters here.)

## Environment notes

- The CH340 console corrupts long multi-line output at 115200. Single-byte
  `i2cget` reads were reliable where a full `i2cdump` grid was garbled;
  this is the same console corruption quirk seen in earlier examples.
- A GDB attach halts the target; an SRST pulse recovers it.
