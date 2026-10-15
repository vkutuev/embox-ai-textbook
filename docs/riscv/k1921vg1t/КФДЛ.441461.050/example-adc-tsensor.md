# Example: internal temperature sensor (ADC)

Read the VG1T's internal temperature sensor through the 12-bit ADC
(channel 63, ADC block 3) and print the temperature. No wiring needed.

> **Status: verified on hardware (2026-10-14)** — `tsensor` reads ~25 °C
> (room temperature), stable across runs. See the
> [experiment journal](experiment-adc-tsensor.md).

## Build and run

1. Write the command module in `<embox>/src/cmds/example/adc_tsensor/`
   (see [Commands and drivers](../../../embox/commands-drivers.md)).
2. Add the `include` line to `mods.conf`:

   ```
   include embox.cmd.example.adc_tsensor.tsensor
   ```
3. Build, flash, reset.
4. On the console:

   ```
   root@embox:/#tsensor 5
   tsensor: ADC ch63 (internal temp sensor), 5 samples
   [1] raw=931  V=750 mV  T=24.7 C
   ...
   ```

The temperature reads room temperature; touch or blow on the MCU to see
it move.

## What the command does

Embox has no ADC driver, so `tsensor` programs the ADC directly:

1. RCU `ADCCFG` (HSE clock, enable, release reset) + `CGCFGAHB`/`RSTDISAHB`.
2. `ACTL[3]`: LDO, range, 12-bit, enable; wait `ADCRDY`.
3. Sequencer 0: software trigger, 3 requests of channel 63; `CHDELAY[63]`.
4. `SEQSYNC.SYNC0` + `SEQEN.SEQEN0`; SIU `TSENSCTRL` (scaled).
5. Per sample: clear `IC`, pulse `GSYNC` (keeping `SYNC0`), wait `BSTAT`
   clear and `SEQRIS0`, read 3 values from `SFIFO`, average.
6. `T = 574.713 · (0.793 − V)` in fixed point, `V = raw/4096 · 3.3 V`.

## Verification checklist (on hardware)

1. `tsensor` prints a temperature near room temperature (~20-30 °C). ✔
2. Repeated runs give a stable value. ✔
3. (Optional) The reading rises if you warm the MCU with a finger.

## Notes

- The absolute temperature assumes `AVCC = 3.3 V` and the on-chip sensor
  calibration; treat it as approximate.
- External ADC channels (for the Sharp 2Y0A21, later) use the same
  sequencer mechanism with a different block/channel number.

## Sources

- ADC chapter: [adc.md](../adc.md).
- SDK `tsensor` example in
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk>.
