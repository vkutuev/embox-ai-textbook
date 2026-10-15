# CRC

A configurable cyclic-redundancy-check engine at `4105_0000h`. It computes
CRC-7/8/16/32 with a programmable polynomial and supports input/output bit
reversal, byte/halfword/word swapping, and an STM32-compatible mode.

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | DR | data input (write a word to feed the CRC) |
| +04h | POST | post-processed CRC value (read) |
| +08h | CR | control: RESET, MODE, XOROUT, POLYSIZE, REV_IN, REV_OUT, SWAP |
| +0Ch | INIT | initial CRC value |
| +10h | POL | programmable polynomial value |

## Configuration

`CR.POLYSIZE` selects a 32-, 16-, 8-, or 7-bit polynomial; `POL` sets the
polynomial itself (defaults are the standard CRC-32/16/8/7 polynomials).
`CR.REV_IN`/`CR.REV_OUT` reverse bit order on input/output, and `CR.SWAP`
reorders the input word (32/16/8-bit or bit string) before calculation.
`CR.MODE` selects standard mode or STM32-compatible mode (XOR of the input),
and `CR.XOROUT` XORs the output.

Usage: set `CR.RESET` to clear the accumulator, write `INIT` if a non-zero
start value is needed, then write data words to `DR`; read the result from
`POST`.

## Embox driver

No Embox driver yet; programmed directly. CRC/HASH is listed as a
zero-hardware candidate example in the board
[development notes](КФДЛ.441461.050/development-notes.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (CRC chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`CRC_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
