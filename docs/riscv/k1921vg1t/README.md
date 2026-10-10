# K1921VG1T

Documentation for the NIET (НИИЭТ) K1921VG1T microcontroller, distilled from
the official user manual, errata, and quick-start guide.

## Contents

- [Overview](overview.md) — MCU features, family, packages
- [Memory map](memory-map.md) — address space and peripheral register map
- [Core (SCR5)](core.md) — ISA, privilege modes, exceptions, MMU/MPU, caches,
  memory-mapped timer
- [Clocks (RCU)](clocks.md) — clock sources, PLL configuration
- [Interrupts (PLIC)](interrupts.md) — interrupt vectors, register map,
  programming model
- [Flash](flash.md) — FlashM/FlashD/OTP, config word, service erase
- [UART](uart.md) — UART register map and baud-rate setup
- [GPIO](gpio.md) — GPIO port registers
- [Debugging](debugging.md) — JTAG, OpenOCD, toolchain, SDK
- [Errata](errata.md) — known silicon issues and workarounds

## Boards

- [КФДЛ.441461.050 (NIIET-DEV-K1921VG1T)](КФДЛ.441461.050/README.md) —
  dev board bring-up with Embox (verified Hello World run), flashing
  guide, and experiment records:
  [SDK OpenOCD configurations](КФДЛ.441461.050/experiment-sdk-openocd-configs.md),
  [GDB `load` flash](КФДЛ.441461.050/experiment-gdb-flash.md)

## Embox relevance

The most important facts for bringing up Embox on K1921VG1T:

| Concern | Answer |
| ------- | ------ |
| ISA flags | Embox template: `-march=rv32imfdc_zicsr_zifencei -mabi=ilp32d` (core is RV32IMAFDCP; the vendor recommends `rv32imafdcp_zicsr`) |
| Boot address | `0x0000_0000` (FlashM) |
| RAM | 1 MiB SRAM at `0x2000_0000` |
| Console UART | UART0 at `0x5201_0000` |
| Interrupt controller | PLIC at `0xFE00_0000`, 254 sources, 16 priority levels |
| Timer | memory-mapped MTIMER (per-core compare), 16 TMR units at `0x5100_8000`+ |
| Debug | JTAG; TCK must satisfy `fSYSCLK / fTCK >= 12` |

The microcontroller has two cores (DualCore or Lockstep mode) and supports
Linux (S-mode with MMU), so Embox can run in supervisor mode.

## Sources

All documents are published in the NIET documentation repository
<https://gitflic.ru/project/niiet/niiet>:

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` — К1921ВГ1Т, К1921ВГ10Я user
  manual (от 25.08.2026)
- `doc/K1921VG1T/Errata_K1921VG1T.pdf` — errata (от 27.02.2026)
- `doc/Start_NIIET_RISCV.pdf` — quick-start guide for NIET RISC-V MCUs
  (от 17.04.2026)
