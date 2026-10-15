# TRNG

A true random-number generator at `5000_8000h` based on ring oscillators, with
a 32-word FIFO, a programmable LFSR seed, and an on-line self-test suite
(repetition, proportion, autocorrelation, and correlation tests).

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CR | control (enable, oscillator select, test enables) |
| +04h | FIFOLEV | FIFO level |
| +08h | FIFOTHR | FIFO interrupt threshold |
| +0Ch | FIFODEP | FIFO depth (read) |
| +10h | KEY[4] | key/seed registers |
| +20h | TESTDATA | test data write |
| +24h | REPTSTCUTOFF | repetition-test cutoff |
| +28h | PROPTESTCUTOFF | proportion-test cutoff |
| +2Ch | LFSRSEED | LFSR seed |
| +30h | STAT | status |
| +34h | WARMPERIOD | warm-up period |
| +38h | DISOSC | oscillator disable |
| +44h | SAMPERIOD | sampling period |
| +58h | HWCFG | hardware configuration (read) |
| +5Ch | COOLDPERIOD | cooldown period |
| +60h | AUTOCORRTESTCUTOFF0/1 | autocorrelation-test cutoffs |
| +68h | CORRTESTCUTOFF0/1 | correlation-test cutoffs |
| +70h | AUTOCORRTESTFAILED | autocorrelation test failed (read) |
| +74h | CORRTESTFAILED | correlation test failed (read) |
| +7Ch | HWVERSION | hardware version (read) |
| +80h | FIFO[32] | random data FIFO |

## Operation

Configure the warm-up (`WARMPERIOD`), sampling (`SAMPERIOD`), and cooldown
(`COOLDPERIOD`) periods, set `FIFOTHR` if using the interrupt, then enable the
generator in `CR`. Random words are read from the `FIFO` (read `FIFOLEV` to
check availability). The self-test cutoffs and failure flags let software
verify the generator's health at runtime.

## Embox driver

No Embox driver yet; programmed directly. TRNG is listed as a
zero-hardware candidate example in the board
[development notes](КФДЛ.441461.050/development-notes.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (TRNG chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`TRNG_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
