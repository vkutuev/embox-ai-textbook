# SDRAM / external memory controller (EMC)

The VG1T has an external memory controller (EMC) that drives the board's
SDRAM. The NIIET-DEV-K1921VG1T board carries **32 MB of SDRAM** mapped at
`8000_0000h`; it is unused until the controller is programmed (the MCU
boots from its 1 MB internal SRAM).

## SDRAM controller registers

Controller base `5000_A000h`.

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | TIMING | `T_RAS`, `T_RP`, `T_RCD`, `T_RC_AR`, `T_WR` (clocks) |
| +04h | REFRESH | `RF_TIME` (period between auto-refresh, clocks) and `RF_MAX` (rows per refresh) |
| +08h | MODE | `BURST_LENGTH`, `BURST_TYPE`, `CAS_LATENCY`, `OPMODE`, `WBMODE` |
| +0Ch | CONTROL | `ENABLE`, `SLOW_BUS`, `DATA_WIDTH` (32-bit), `COLUMN_WIDTH` (9-bit), `REQ_DEPTH` |
| +10h | STATUS | `INIT_DONE` |

## Clock and enable

- SDRAM clock source is selected by `RCU.SDRAMCFG` (`9Ch`): `CLKSEL`
  (PLL0 = 0), `CLKEN`, `DIVEN`/`DIVN`, `RSTDIS`. The SDK intends ~90 MHz.
- Enable the peripheral clock with `RCU.CGCFGAPB0.SDRAMEN` and release
  reset with `RCU.RSTDISAPB0.SDRAMEN` (bit 5).
- Route the SDRAM controller to the external pins with
  `SIU.EMMUXCTRL.SDRAMEN = 1` (0 = EMC/NOR, 1 = SDRAM).
- The SDRAM pins (ports D/E/F/G: address, data, bank, DQM, WE/RAS/CAS/CS,
  CLK, CKE) must be muxed to alt function 2 and the port clocks enabled.

## Embox note

Embox has no SDRAM/EMC driver. The
[SDRAM example](КФДЛ.441461.050/example-sdram.md) programs the controller
directly (following the NIIET SDK `bsp`) and verifies the full 32 MB with
a write/read test. Registering the SDRAM as a `phymem` region (so the
heap actually uses it) is a separate step not yet done.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (EMC/SDRAM chapter).
- SDK `hardware/bsp/NIIET-DEV-K1921VG1T/bsp.c` and the `sdram` project in
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk>.
