# Experiment: SDRAM bring-up (2026-10-14)

Hardware verification of the [SDRAM example](example-sdram.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console). No external hardware.

## Result

**Verified.** `sdram` initializes the SDRAM controller (32 MB at
`0x80000000`) and the full write/read test passes: `sdram: OK (32 MB)`.

## Findings

- Embox has no SDRAM/EMC driver, so the command follows the NIIET SDK
  `bsp.c` sequence directly:
  1. mux SDRAM pins (ports D/E/F/G) to alt function 2 + enable port clocks;
  2. `RCU.CGCFGAPB0`/`RSTDISAPB0.SDRAMEN`, `SIU.EMMUXCTRL.SDRAMEN = 1`
     (SDRAM mode), `RCU.SDRAMCFG` (PLL0, `CLKEN`/`DIVEN`);
  3. `MODE` (CAS latency 2), `CONTROL` (9-bit column, 32-bit data, req
     depth 3), `REFRESH` (RF_MAX 2, RF_TIME 120), then `ENABLE`;
  4. wait `STATUS.INIT_DONE`, then write/read all 32 MB.
- The 32 MB write/read test takes ~10 s (uncached volatile accesses);
  it completes with zero errors.
- This only verifies the memory. Registering the SDRAM as an Embox
  `phymem` region (so the heap actually uses the extra 32 MB) is not yet
  done and would be a follow-up.

## Environment notes

- A GDB attach halts the target; an SRST pulse recovers it.
- The CH340 console corrupts long output; short command output is
  reliable.
