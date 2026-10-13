# I2C

Four identical I2C controllers (I2C0-I2C3), master and slave operation,
standard (100 kHz), fast (400 kHz), fast-plus (1 MHz), and high-speed
(3.4 MHz) modes, 7-bit or 10-bit addressing, clock stretching, and up to
eight extended-address match registers for multi-slave filtering.

## Register map

Base addresses:

```
I2C0  5201_8000h   I2C1  5201_9000h   I2C2  5201_A000h   I2C3  5201_B000h
```

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | SDA | data: write = TX byte, read = RX byte |
| +04h | ST | status: INT flag and current state code (`ST.MODE`) |
| +08h | CST | status/control: BB (bus busy), TOERR (timeout), ... |
| +0Ch | CTL0 | control: START, STOP, INTEN, ACK (send NAK), CLRST |
| +10h | ADDR | own address (slave mode) |
| +14h | CTL1 | control: ENABLE and low 7 bits of the SCL frequency divider |
| +18h | TOPR | prescaler load value |
| +1Ch | CTL2 | high-speed mode SCL divider (low bits) |
| +20h | CTL3 | high 8 bits of the SCL frequency divider |
| +24h | CTL4 | high-speed mode SCL divider (high bits) |
| +28h | CTL5 | reserved for additional control |
| +2Ch | CTL6 | reserved for additional control |
| +30h | EADDR[0..7] | extended address match registers |

The controller is a state machine: the CPU writes a byte to `SDA`, the
controller shifts it out on the bus, then signals completion in `ST.INT`
and reports the result in `ST.MODE` (e.g. `MTDAPA` = master TX data
acknowledged, `MTADNA` = slave address NAK). Software drives the bus by
reading `ST.MODE` after each step. This is the model Embox's driver uses
(see below).

## Embox driver

The Embox `k1921vg1t_dev` template uses the `vg015_i2c` driver
(`src/drivers/i2c/niiet/vg015_i2c/`), shared with the K1921VG015. The
board config (`platform/niiet/board_config/niiet_k1921vg1t_dev.conf.h`)
already defines I2C0 (pins PD3 = SCL / PD2 = SDA, alt function 2, clock
`CLK_I2C0`) and I2C1 (PD1 / PD0, alt function 1); enabling
`embox.driver.i2c.niiet.vg015_i2c0` registers bus 0, configures the pins
open-drain with the internal pull-up, and enables the clock.

The driver is interrupt-free and polling-based (`i2c_master_xfer`), so it
works regardless of how IRQs are set up. Two details to keep in mind:

- `I2C_SYSTEM_FREQ` is hardcoded to 50 MHz for the SCL divider. If the
  actual `CLK_I2C0` differs, the resulting SCL is scaled by the same
  factor (slower or faster than the 100 kHz target).
- A NAK on the device address is normal during `i2cdetect` scanning and is
  returned as `-ENXIO`, not treated as an error.

Board usage:
[PCA9685 example](КФДЛ.441461.050/example-i2c-pca9685.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (I2C chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` in
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk> (`I2C_TypeDef`).
