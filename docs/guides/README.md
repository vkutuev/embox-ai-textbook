# Guides

Architecture-agnostic learning material for newcomers. These pages introduce
the concepts a student needs before (or while) bringing up Embox on the
K1921VG1T. They are intentionally not board-specific; board- and MCU-specific
facts live under `docs/riscv/k1921vg1t/`.

## Contents

- [Git basics](git-basics.md) — the minimal Git needed to clone and work with
  Embox and the NIIET SDKs
- [Cross-compilation](cross-compilation.md) — host vs. target, toolchain
  triples, and the toolchain options for the K1921VG1T
- [Embedded C](embedded-c.md) — memory-mapped I/O, `volatile`, bit operations,
  register maps
- [JTAG, OpenOCD, GDB](jtag-openocd-gdb.md) — how the debug toolchain fits
  together
- [RISC-V basics](riscv-basics.md) — registers, CSRs, privilege modes, and the
  calling convention

## Prerequisites (learning order)

| To study | You should already be able to |
| --- | --- |
| Embox build system | Git basics; cross-compilation |
| Commands and drivers | Embedded C (MMIO, `volatile`, read-modify-write) |
| Interrupts (PLIC) | RISC-V basics (CSRs, privilege modes); a working command |
| GPIO / timer examples | Memory map; the `pin` shell command |
| Any driver example | Commands and drivers; the board README (wiring/reset) |
| Debugging on hardware | JTAG/OpenOCD/GDB; the debug checklist |
