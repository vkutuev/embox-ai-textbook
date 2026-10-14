# Experiment: internal temperature sensor (ADC) (2026-10-14)

Hardware verification of the
[ADC temperature-sensor example](example-adc-tsensor.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console). No external hardware.

## Result

**Verified.** `tsensor` reads the internal temperature sensor (ADC
channel 63, block 3) and reports ~25 °C (room temperature):
`raw=930`, `V=750 mV`, `T=24.7-25.8 °C`, stable across runs.

## Findings

- Embox has no ADC driver and the board config has no ADC entry, so the
  command programs the registers directly (RCU `ADCCFG`, `ACTL[3]`,
  sequencer 0, SIU `TSENSCTRL`), following the SDK `tsensor` example.
- ADC base `0x41020000`; the temperature sensor is channel 63 on block 3.
  Temperature `T = 574.713 · (0.793 − V)` for the scaled output
  (`SCALE=1`), with `V = raw/4096 · 3.3 V`.
- **`SEQSYNC` overwrite bug**: triggering by writing only `GSYNC`
  (bit 31) clears `SYNC0` (bit 0), disabling the sequencer's software
  sync — the sequencer then never converts. Fixed by writing
  `SYNC0 | GSYNC`. The SDK uses a bit-field write that preserves `SYNC0`.
- The read path is polling (no IRQ attach), so there is no re-run leak
  (unlike the DMA/GPIO/timer IRQ examples).

## Environment notes

- The absolute temperature assumes `AVCC = 3.3 V` and the on-chip sensor
  calibration; the ~25 °C reading matches room temperature, so the
  assumption is reasonable for this board.
- A GDB attach halts the target; an SRST pulse recovers it.
