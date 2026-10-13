# PCA9685 PWM example for NIIET-DEV-K1921VG1T

Drive an NXP PCA9685 16-channel 12-bit PWM controller over I2C0, and
observe the generated PWM on a logic analyzer. This is the textbook's I2C
example: bring up a second peripheral on the I2C bus and write a small
register-level device driver.

## Hardware

| Board | Signal | PCA9685 module |
| ----- | ------ | -------------- |
| XP1 pin 5 | PD2 = I2C0_SDA | **SDA** |
| XP1 pin 3 | PD3 = I2C0_SCL | **SCL** |
| XP1 pin 2 (any GND) | GND | **GND** |
| XP3 pin 19 | +3.3 V | **VCC** (logic) and **V+** (outputs) |

- Default I2C address **0x40** (address-select jumpers A0–A5 all low).
- Logic at 3.3 V (the MCU pins are not 5 V-tolerant).
- The board's I2C0 lines (PD2/PD3) are configured open-drain with the
  MCU's internal pull-ups by the Embox driver; a GY-PCA9685-style module
  adds its own 10 kΩ pull-ups.
- Probe any PWM output (PWM0–PWM15) with the logic analyzer; connect the
  analyzer GND to the common ground.

## Install into Embox

1. Copy this directory into an Embox checkout:

   ```console
   $ cp -r pca9685/ <embox>/src/cmds/example/
   ```

2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the line from
   `mods.conf.snippet` (`embox.cmd.example.pca9685.pca9685`). The I2C0
   bus and the i2c-tools commands are already in the template.

3. Build, flash, reset.

## Run and verify

1. On the console, confirm the bus and the device:

   ```
   root@embox:/#i2cdetect -l
   i2c_bus0
   root@embox:/#i2cdetect -y 0
   ... 40 ...
   ```

2. Configure the PWM frequency and a channel, then watch the output:

   ```
   root@embox:/#pca9685 init 1000
   root@embox:/#pca9685 set 0 2048     # 50% duty on PWM0
   root@embox:/#pca9685 set 0 1024     # 25% duty
   root@embox:/#pca9685 info
   ```

   On the logic analyzer (PWM0) you should see a ~1 kHz square wave whose
   duty cycle follows the `off` value.
