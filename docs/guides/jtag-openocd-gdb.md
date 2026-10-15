# JTAG, OpenOCD, GDB

The debug and flashing toolchain for the K1921VG1T has three layers. This
page explains what each does and how they fit together.

## JTAG

JTAG is a hardware interface for debugging and programming. It exposes an
on-chip **debug module** through a serial scan chain, so a host can halt the
CPU, read/write registers and memory, and drive flash programming. The
K1921VG1T JTAG port has five signals — TCK, TMS, TDI, TDO, TRST — and supports
J-Link, CMSIS-DAP, Olimex, and FT2232 adapters.

The debug module is independent of the running program, which is why you can
flash a chip even when no program (or a broken one) is running.

## OpenOCD

**OpenOCD** is the host software that talks to the JTAG adapter. It is
configured by scripts that describe:

- the **adapter** (interface), e.g. `interface/jlink.cfg`;
- the **target** (the MCU and its debug module), e.g.
  `target/k1921vg1t_singlecore.cfg`;
- **flash banks** and any custom flash drivers.

OpenOCD exposes a **telnet** console (default port 4444) and a **GDB server**
(port 3333). From the telnet console you run the flashing commands directly
(`halt`, `flash erase_sector`, `flash write_image`, `verify_image`, `resume`).

The stock OpenOCD does not know the K1921VG1T flash controller, so you must
build the NIIET fork (<https://gitflic.ru/project/niiet/openocd>) and use the
per-MCU configs from the NIIET SDK. See
[Debugging](../riscv/k1921vg1t/debugging.md) and the board
[README](../riscv/k1921vg1t/КФДЛ.441461.050/README.md).

## GDB

**GDB** is the source-level debugger. It connects to OpenOCD's GDB server with
`target extended-remote :3333`, then provides breakpoints, single-stepping,
and register/memory inspection:

```console
$ riscv64-unknown-elf-gdb build/base/bin/embox
(gdb) target extended-remote :3333
(gdb) break main
(gdb) continue
```

GDB itself knows nothing about JTAG; OpenOCD translates GDB's abstract
commands into JTAG operations.

## The three-layer model

```
GDB  ──TCP 3333──▶  OpenOCD  ──adapter driver──▶  JTAG adapter  ──signals──▶  MCU debug module
                       │
                       └── telnet 4444 ──▶ operator (flashing commands)
```

Typical workflows:

- **Flashing**: use the OpenOCD telnet console directly.
- **Debugging**: use GDB over OpenOCD.
- **Automation**: drive OpenOCD from scripts/`-c` commands.

## Reset caveats on this chip

The K1921VG1T debug module has quirks that matter in practice: `reset init`
times out, so use a manual SRST pulse or `ndmreset`; after a hart-only reset
the UART clock domain can break, so always do a full system reset before
running a freshly flashed image. Details and the verified procedure are in the
board [README](../riscv/k1921vg1t/КФДЛ.441461.050/README.md).

## Sources

- [OpenOCD documentation](https://openocd.org/doc/html/index.html)
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb/)
- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 41
