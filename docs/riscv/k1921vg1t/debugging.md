# Debugging and Toolchain

## JTAG

The JTAG port consists of five pins: TCK, TMS, TDI, TDO, TRST. Supported
adapters: Olimex ARM-USB-OCD-H, J-Link V9, CMSIS-DAP, FT2232-based
debuggers.

Verified debug-architecture facts (from the SDK configs and real
hardware):

- Both cores have their own TAP: IDCODE `0xDEB15001`, IR length 5.
- In LOCKSTEP mode only one debug module exposes a hart (hart index max
  is 0); dualcore OpenOCD configs fail with "Hart index 1 is too large"
  / "unavailable". Use a single-target config (`-coreid 0`), e.g.
  `target/k1921vg1t_singlecore.cfg`, or switch the chip to DUALCORE
  (see [core.md](core.md)).
- The debug module supports `ndmreset` (bit 1 of `dmcontrol`, DMI address
  `0x10`): a full system reset from the debugger. In OpenOCD:
  `riscv dmi_write 0x10 0x80000003` then `riscv dmi_write 0x10 0x80000001`.
  This is the reliable reset method when SRST is not wired.
- Without a working SRST, `reset init`/`reset halt` time out with
  "Hart didn't leave reset in 5s; dmstatus=0x400382". Flashing works
  with the target simply halted; to run the image, reset via `ndmreset`
  and resume from the reset vector.
- Flashing via GDB (`load`) requires overriding the default flash events
  (they run `reset init`):
  `configure -event gdb-flash-erase-start { halt }` and
  `configure -event gdb-flash-write-end { halt }`.

The controller always starts clocked from the internal RC oscillator
(~4 MHz), so the following ratio must be observed between the system clock
and the JTAG clock:

```
fSYSCLK / fTCK >= 12
```

For the K1921VG1T the JTAG frequency must not exceed 200-250 kHz. Debug can
be disabled with the JTAGEN bit of the configuration word CFGWORD.

If the debug session cannot start, possible causes are:

1. Missing internal LDO voltage (check VCORE_CAP/VBAT_CAP pins).
2. JTAG disabled in CFGWORD or a low-power program already flashed.

Both cases are solved by service full erase of the flash (SERVEN high
during reset, then four `SERVCMD` writes via JTAG).

## Software

- Toolchain: RISC-V GCC (NIET recommends the xpack `riscv-none-elf-gcc` or
  Syntacore Development Toolkit). Flags for K1921VG1T:

  ```
  -march=rv32imafdcp_zicsr -mabi=ilp32d
  ```

- SDK and examples: NIIET_RISCV SDK —
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk.git> (branch `master`),
  plus per-MCU example repositories. Contains device headers, startup
  files, linker scripts (`platform/Device/...`), examples, templates, SVD
  files, and OpenOCD scripts.
- OpenOCD with K1921VG1T flash driver:
  <https://gitflic.ru/project/niiet/openocd/release>.
- IDE: Syntacore IDE (Eclipse-based, Windows/Linux) or VS Code with the
  `niiet-aspect` extension (VSIX from the SDK tools directory).
- Debug session with OpenOCD: `-f <adapter>.cfg -f k1921vg1t.cfg
  -c "init;halt"`; connect GDB with `target extended-remote :3333`.
- Linker scripts: separate scripts for building for RAM or flash
  (e.g. `k1921vg015_ram.ld`, `k1921vg015_flash.ld` in `platform/ldscripts`).
- Typical crystal frequencies for PLL divider selection in examples:
  10, 12, 16, 20, 24 MHz.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 41
- `doc/Start_NIIET_RISCV.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, sections 2-6
