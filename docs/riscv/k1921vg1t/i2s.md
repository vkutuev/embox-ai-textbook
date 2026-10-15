# I2S

Two identical I2S/audio serial interfaces (I2S0, I2S1) with separate
transmit and receive FIFOs, a master-clock divider, and configurable frame
format (clock dividers and frame control).

## Register map

Base addresses:

```
I2S0  5100_6000h   I2S1  5100_7000h
```

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | I2SCTRL | control (enable, mode, data width, FIFO config) |
| +04h | I2SCLKDIV1 | clock divider 1 |
| +08h | I2SCLKDIV2 | clock divider 2 |
| +0Ch | I2SFRMCTRL1 | frame control 1 (slot/word length, format) |
| +10h | I2SFRMCTRL2 | frame control 2 |
| +14h | I2SRECBUFUSED | receive buffer count (read) |
| +18h | I2STRANSDATA | transmit data |
| +1Ch | I2SMCLKDIV | master-clock divider |
| +20h | I2SRECDATA | receive data (read) |
| +24h | I2SSHIFTDATA | shift data register |
| +28h | I2SSTATE | FIFO status (TXFIFO_STATE, RXFIFO_STATE) |
| +2Ch | I2SSTATECLR | FIFO status clear |
| +30h | I2STRBUFUSED | transmit buffer count (read) |

## Operation

The sample rate is set via `I2SMCLKDIV` (master clock) and
`I2SCLKDIV1`/`I2SCLKDIV2`; the frame layout (word length, slot count, and
format) is configured in `I2SFRMCTRL1`/`I2SFRMCTRL2`. Samples are written to
`I2STRANSDATA` and read from `I2SRECDATA`; the `*BUFUSED` counters and
`I2SSTATE` flags track FIFO occupancy.

## Embox driver

No Embox driver yet; programmed directly. I2S is listed as a candidate
example in the board
[development notes](КФДЛ.441461.050/development-notes.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (I2S chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`I2S_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
