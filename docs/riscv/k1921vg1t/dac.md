# DAC

Two identical 12-bit digital-to-analog converters (DAC0, DAC1) with an
output buffer, an internal adjustable bandgap reference, software and
hardware triggers, DMA, and a pattern generator (triangle, sine, meander).

## Register map

Base addresses:

```
DAC0  5100_4000h   DAC1  5100_5000h
```

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | DATA12 | 12-bit output value |
| +04h | DATA8 | 8-bit output value (4 LSBs forced to 0) |
| +08h | DATA_DUAL | simultaneous value for DAC0 and DAC1 (DAC0 only) |
| +0Ch | CONTROL | DISABLE, EXT_REF, BUF_EN, TSEL (trigger select), WAVE (wave gen), DATAMODE, SIGN, MASK (amplitude), SCALE, MULDIV, NEGATE, OFFSET |
| +10h | BGCTRL | internal bandgap reference control |
| +14h | DMACTRL | DMA control |
| +18h | SWTRIG | software trigger |
| +1Ch | HWTRIG | hardware trigger generator |
| +20h | PATDORL | pattern data (patterns 0-5) |
| +24h | PATDORH | pattern data (patterns 6-8) |
| +28h | TEST | test output selection |

## Operation

Write the output code to `DATA12` (right-aligned) or `DATA8`. `CONTROL` sets
the reference (`EXT_REF` for an external voltage, otherwise the internal
bandgap), enables the output buffer (`BUF_EN`), and selects the conversion
trigger (`TSEL`): software (`SWTRIG`) or a hardware event. `DATAMODE`
switches between independent channels and simultaneous (`DATA_DUAL`) update.

`CONTROL.WAVE` + `MASK` enable wave generation: the DAC outputs a triangle,
sine, or meander pattern from `PATDORL`/`PATDORH`, useful for testing without
CPU involvement. `SCALE`/`MULDIV`/`NEGATE`/`OFFSET` scale and bias the data
before conversion.

## Embox driver

No Embox driver yet; programmed directly. A DAC example needs a scope or DMM
to verify the analog output (listed as a candidate example in the board
[development notes](КФДЛ.441461.050/development-notes.md)).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (DAC chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`DAC_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
