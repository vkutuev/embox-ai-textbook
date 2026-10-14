# SDRAM example for NIIET-DEV-K1921VG1T

Bring up the board's 32 MB external SDRAM (mapped at 0x80000000) and
verify it with a write/read test. No wiring needed.

## Install into Embox

1. Copy `sdram/` into `<embox>/src/cmds/example/`.
2. Add the `include` from `mods.conf.snippet` to `mods.conf`.
3. Build, flash, reset.

## Run and verify

```
root@embox:/#sdram
sdram: bringing up 32 MB SDRAM @ 0x80000000
sdram: controller initialized
sdram: testing 32 MB ...
sdram: OK (32 MB)
```

## How it works

Embox has no SDRAM driver, so the command follows the NIIET SDK `bsp`
sequence:

1. Mux all SDRAM signals to alt function 2 (ports D/E/F/G: address, data,
   BA, DQM, WE/RAS/CAS/CS, CLK, CKE) and enable the port clocks.
2. Enable the SDRAM clock: `RCU.CGCFGAPB0`/`RSTDISAPB0.SDRAMEN`, SIU
   `EMMUXCTRL.SDRAMEN` (route SDRAM to the external pins), `RCU.SDRAMCFG`
   (PLL0 source, enable).
3. Configure the controller: `MODE` (CAS latency 2), `CONTROL`
   (9-bit column, 32-bit data, req depth 3), `REFRESH` (RF_MAX 2,
   RF_TIME 120), then `ENABLE`.
4. Wait `STATUS.INIT_DONE`, then write a pattern across all 32 MB and
   read it back.

## Note

This example only verifies the memory; it does not yet register the SDRAM
as an Embox physical-memory region / heap (that needs a `phymem` entry).
