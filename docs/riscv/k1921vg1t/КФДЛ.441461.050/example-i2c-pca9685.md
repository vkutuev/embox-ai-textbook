# Example: PCA9685 PWM driver over I2C

Drive an NXP PCA9685 16-channel, 12-bit PWM controller over the board's
**I2C0** bus. This is the textbook's I2C chapter: bring up a second
peripheral on the I2C bus and write a small register-level device driver.
The generated PWM is observed with a logic analyzer (no LEDs or servos
needed).

> **Status: verified on hardware (2026-10-13)** — the device responds on
> I2C0 at `0x40` and the PWM was observed in PulseView at 1.1 kHz. See the
> [experiment journal](experiment-i2c-pca9685.md).

## Hardware

| Board | Signal | PCA9685 module |
| ----- | ------ | -------------- |
| XP1 pin 5 | PD2 = I2C0_SDA | **SDA** |
| XP1 pin 3 | PD3 = I2C0_SCL | **SCL** |
| XP1 pin 2 (any GND) | GND | **GND** |
| XP3 pin 19 | +3.3 V | **VCC** (logic) and **V+** (outputs) |

- Default I2C address **0x40** (address-select jumpers A0-A5 all low).
- Logic at **3.3 V** (the MCU pins are not 5V-tolerant). Power `V+` from
  3.3 V too, so the PWM outputs swing 0-3.3 V and stay safe for a 3.3 V
  logic analyzer.
- The board's I2C0 lines are configured open-drain with the MCU's
  internal pull-up by the Embox driver; a GY-PCA9685-style module adds its
  own 10 kΩ pull-ups.

## Embox support

The `k1921vg1t_dev` template already includes everything on the I2C side:

```
include embox.driver.i2c.niiet.vg015_i2c0(base_addr=0x52018000)
include embox.cmd.i2c_tools.i2cdetect     # plus i2cdump/i2cget/i2cset
```

`vg015_i2c0` registers bus 0, configures PD2/PD3 (alt function 2)
open-drain with internal pull-up, and enables `CLK_I2C0`. The example only
adds the command:

```
include embox.cmd.example.pca9685.pca9685
```

The `pca9685` command uses the i2c-core API (`i2c_bus_transfer` /
`i2c_bus_write` in `<drivers/i2c/i2c.h>`) and implements four
subcommands: `info` (read back MODE1/MODE2/PRE_SCALE), `init` (set the PWM
frequency via the PRE_SCALE divider), `set` (per-channel duty), `all`
(all channels).

## Build and run

1. Write the command module in `<embox>/src/cmds/example/pca9685/` (see
   [Commands and drivers](../../../embox/commands-drivers.md)).

2. Add the `include embox.cmd.example.pca9685.pca9685` line to
   `platform/niiet/templates/k1921vg1t_dev/mods.conf`.

3. Build, flash, reset.

4. On the console, confirm the bus and the device:

   ```
   root@embox:/#i2cdetect -l
   i2c_bus0
   root@embox:/#i2cdetect -y 0
   ... 40 ...
   ```

5. Configure and verify (logic analyzer on PWM0):

   ```
   root@embox:/#pca9685 init 1000
   root@embox:/#pca9685 set 0 2048     # 50% duty
   root@embox:/#pca9685 set 0 1024     # 25% duty
   root@embox:/#pca9685 info
   ```

## Verification checklist (on hardware)

1. `i2cdetect -y 0` reports the device at `0x40`. ✔ (2026-10-13)
2. `pca9685 info` reads back MODE1/MODE2/PRE_SCALE (device responds). ✔
3. Logic analyzer on PWM0 shows a square wave. ✔ — measured **1.1 kHz**
   for `init 1000` (the PCA9685's internal oscillator runs ~27 MHz, not
   the nominal 25 MHz; see the experiment journal).
4. Changing `off` changes the duty cycle (verified by reading back the
   LED0_OFF register: `0x0800` for 2048, `0x0400` for 1024). ✔
5. SCL frequency not measured — the driver's `CLK_I2C0 = 50 MHz`
   assumption is still unverified.

## Sources

- I2C peripheral chapter: [i2c.md](../i2c.md).
- PCA9685 data sheet: <https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf>.
