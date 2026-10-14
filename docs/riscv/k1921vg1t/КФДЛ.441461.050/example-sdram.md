# Example: SDRAM bring-up

Bring up the board's 32 MB external SDRAM and verify it with a write/read
test. No wiring needed.

> **Status: verified on hardware (2026-10-14)** — `sdram` initializes the
> controller and reports `OK (32 MB)` (the full write/read passes). See
> the [experiment journal](experiment-sdram.md).

## Build and run

1. Copy `examples/niet-dev-k1921vg1t/sdram/` into `<embox>/src/cmds/example/`.
2. Add the `include` from `mods.conf.snippet` to `mods.conf`.
3. Build, flash, reset.
4. On the console:

   ```
   root@embox:/#sdram
   sdram: bringing up 32 MB SDRAM @ 0x80000000
   sdram: controller initialized
   sdram: testing 32 MB ...
   sdram: OK (32 MB)
   ```

The 32 MB test takes a few seconds (uncached volatile accesses).

## What the command does

Following the NIIET SDK `bsp`, the command:

1. Muxes all SDRAM signals to alt function 2 (ports D/E/F/G) and enables
   their clocks.
2. Enables the SDRAM clock: `RCU.CGCFGAPB0`/`RSTDISAPB0.SDRAMEN`,
   `SIU.EMMUXCTRL.SDRAMEN` (SDRAM mode), `RCU.SDRAMCFG` (PLL0, enable).
3. Configures `MODE` (CAS latency 2), `CONTROL` (9-bit column, 32-bit
   data, req depth 3), `REFRESH`, then `ENABLE`.
4. Waits `STATUS.INIT_DONE`, then writes `i | 0xA5A50000` to every word
   and reads it back.

## Verification checklist (on hardware)

1. `sdram` reports `controller initialized` (INIT_DONE set). ✔ (2026-10-14)
2. `sdram` reports `OK (32 MB)` (write/read passes). ✔

## Note

This only verifies the memory; it does not register the SDRAM as an
Embox `phymem` region, so the heap still uses the 1 MB internal SRAM.
Adding a `phymem` entry would make the extra 32 MB usable.

## Sources

- SDRAM chapter: [sdram.md](../sdram.md).
- SDK `sdram` project and `bsp.c` in
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk>.
