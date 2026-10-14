# ADC

The VG1T has a successive-approximation ADC (base `4102_0000h`) with four
independent blocks (`ACTL[0..3]`) and 64 input channels (16 per block).
It resolves 6/8/10/12 bits and measures channels via up to eight
sequencers, each of which converts a programmed list of channels into a
FIFO. The internal temperature sensor is wired to **channel 63 (block
3)**.

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | SEQEN | sequencer enable (bit n = sequencer n) |
| +04h | SEQSYNC | `SYNC0..7` (bits 0-7, software-sync enable) and `GSYNC` (bit 31, trigger all) |
| +0Ch | BSTAT | busy status |
| +1Ch | EMUX | start-event select per sequencer (`EM0` for seq 0; 0 = software) |
| +20h | RIS | raw interrupt status (`SEQRIS0` = sequencer 0 done) |
| +2Ch | IC | interrupt clear |
| +40h + 3Ch·n | SEQ[n] | sequencer n: `SRQSEL0..5` (+00..14, channel per request), `SRQCTL` (+18, `RQMAX`), `SCCTL` (+24, `ICNT`/`RCNT`), `SRTMR` (+30), `SFLOAD` (+34), `SFIFO` (+38) |
| +540h + 10h·n | ACTL[n] | block control: `ADCEN`, `ADCRDY`, `ADCENLDO`, `ADCSTART`, `SELRES` (12-bit = 3), `RANGELDO`, `DISLVL` |
| +680h + 4·n | CHDELAY[n] | per-channel sampling delay |

## Clock

`RCU.ADCCFG` (offset `90h`): `CLKSEL` (0 = HSI, 1 = HSE), `CLKEN`,
`RSTDIS`, `DIVEN`/`DIVN`. Enable the AHB gate with
`RCU.CGCFGAHB.ADCEN` and release reset with `RCU.RSTDISAHB.ADCEN`
(both bit 10). The dev board uses HSE (27 MHz).

## Internal temperature sensor

- Enable with SIU `TSENSCTRL` (`4Ch`): `SCALE` (bit 1) and `DIS` (bit 0).
- Read channel 63 on block 3. With `SCALE=1`, voltage V (from
  `raw/4096 · AVCC`) converts to temperature as `T = 574.713 · (0.793 − V)`
  °C (with `SCALE=0`: `T = 280.899 · (1.594 − V)`).

## Embox note

Embox has no ADC driver, so the [temperature-sensor example]
(КФДЛ.441461.050/example-adc-tsensor.md) programs the registers directly
(RCU, `ACTL[3]`, sequencer 0, SIU `TSENSCTRL`). A sequencer conversion is
triggered by pulsing `GSYNC` while keeping `SYNC0` set (writing only
`GSYNC` clears `SYNC0` and disables the trigger).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (ADC chapter and appendix A).
- SDK `projects/NIIET-DEV-K1921VG1T/tsensor` and `.../adc` in
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk>.
