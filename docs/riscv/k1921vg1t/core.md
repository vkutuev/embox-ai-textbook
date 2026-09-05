# Core (SCR5)

The K1921VG1T is based on the dual-core SCR5 RISC-V CPU.

## ISA and pipeline

- Instruction set: RV32IMAFDCP — integer (Zicsr, Zifencei), multiply/divide,
  atomic, single/double-precision floating point, compressed (C), and P
  (DSP/SIMD) extensions.
- 7- to 9-stage pipeline; Harvard architecture with separate instruction and
  data paths; little-endian.
- Load/store architecture, 32-bit address space with byte access.
- High-performance multiply/divide unit; FPU compliant with IEEE 754-2008
  (32 FP registers).
- Inter-core communication: SCRX (per-core machine software interrupts and
  short message exchange), ICCM module.
- Lockstep mode available (LOCKSTEPEN in CFGWORD); DualCore is the
  alternative mode.

## Lockstep vs dualcore

The two cores run in one of two modes, selected at power-on reset by the
`LOCKSTEPEN` bit of the configuration word (default: LOCKSTEP):

- **LOCKSTEP** — one core is the working core, the second is a shadow
  checker executing the same program. A comparison module compares the
  output buses of both cores (nodes IC, DC, L1I, L1D, SMP); on mismatch it
  signals an error to the SIU (self-testing is available). The working
  core's outputs are delayed by 2 clock cycles, the checker's inputs by
  2 clock cycles. PLIC signals and the L2 interface are not compared.
  From the software and debug perspective the pair is a **single logical
  hart**: the debug module exposes only hart 0.
- **DUALCORE** — the cores run independently, each with its own debug
  module and its own hart (`mhartid` 0 and 1). This is the mode for SMP
  use (Embox supports RISC-V SMP: `embox.arch.riscv.kernel.smp`,
  `embox.kernel.cpu.smp`, see `templates/riscv/smp`).

Runtime switching is allowed if `SWMODEEN` and `LOCKSTEPEN` in CFGWORD
permit it: write `1` to the `DUALCORE` field of the SIU `CORE1_CONF`
register (offset +1Ch, key `0xACCE55` in `CODE[31:8]`); this resets the
MCU and sets the `LSTEP` bit in the RCU `RSTSTAT` register. The `RVECT`
register (offset +18h) holds the reset vector of core 1 in DUALCORE mode.

Consequences for debugging and flashing:

- In LOCKSTEP mode dualcore OpenOCD configs (two targets with coreid 0
  and 1) fail — the second hart does not exist ("Hart index 1 is too
  large"). A single-target config with `-coreid 0` works.
- In DUALCORE mode both harts are visible and controllable
  independently or as an SMP group.
- Service mode (SERVEN=1 during reset) makes the CFGWORD read as zeros,
  so the chip starts in DUALCORE mode — see
  [КФДЛ.441461.050](КФДЛ.441461.050/README.md) for the flashing
  procedure.

## Privilege levels

The core supports three privilege modes: machine (M), supervisor (S), and
user (U). This allows running Linux-class systems (Embox included) in
S-mode.

## Exceptions and interrupts

M-mode exception codes (mcause): 0 instruction address misaligned,
1 instruction access fault, 2 illegal instruction, 3 breakpoint,
4 load address misaligned, 5 load access fault, 6 store/AMO address
misaligned, 7 store/AMO access fault, 8 environment call from U-mode,
9 environment call from S-mode, 11 environment call from M-mode,
12 instruction page fault, 13 load page fault, 14 TLB miss,
15 store/AMO page fault.

Interrupt codes: 1 supervisor software interrupt, 3 machine software
interrupt, 5 supervisor timer interrupt, 7 machine timer interrupt,
9 supervisor external interrupt, 11 machine external interrupt,
31 memory-fault.

Interrupt priority (highest first): memory fault, machine external, machine
software, machine timer, supervisor external, supervisor software,
supervisor timer.

Interrupt CSR registers: `MIE/MIP/MCAUSE` in M-mode, `SIE/SIP/SCAUSE` in
S-mode. PLIC asserts MEIP (bit 11 of `mip`) or SEI (bit 9).

## Caches and MMU

- L1 instruction cache 32 KiB, L1 data cache 32 KiB, coherent L2 cache
  128 KiB.
- MMU with paged virtual memory: 32-entry ITLB, 32-entry DTLB.
- MPU with 16 entries.
- On a TLB miss, the handler walks page tables and writes the PTE to
  `TLB_ATTR` with zero to `TLB_UPDATE`; CSR `SWPW_R0_ADDR` contains the
  precomputed root-page-table address, `SWPW_R1_ADDR` the second-level PTE
  offset.

## Memory-mapped machine timer (MTIMER)

A shared 64-bit counter plus a per-core 64-bit compare register that
generates a machine timer interrupt on match. Register map relative to a
user-defined `MTIMER_BASE` (which must be configured in the MPU with
`MPU_CONTROL.MTYPE=3`):

| Offset | Register |
| ------ | -------- |
| 0x0 | MTIMER_CONTROL (ENABLE, CLKSRC) |
| 0x4 | MTIMER_DIVIDER (0 = every cycle, N = every N+1 cycles) |
| 0x8 | MTIMER_MTIMELO |
| 0xC | MTIMER_MTIMEHI |
| 0x10 | MTIMER_MTIMECMPLO (per-core) |
| 0x14 | MTIMER_MTIMECMPHI (per-core) |

Accesses must be uncached, strongly ordered, 32-bit aligned loads/stores;
otherwise the transaction is ignored and a bus error is returned.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 12
