# RISC-V Basics

The K1921VG1T core (SCR5) implements RV32IMAFDCP, a 32-bit RISC-V ISA. This
page covers the register set, CSRs, and privilege modes you need to read
startup code, disassembly, and bare-metal examples.

## Registers

RISC-V has 32 general-purpose integer registers `x0`–`x31`, plus a program
counter `pc`. Each register has an ABI name used in assembly and calling
conventions:

| Register | ABI name | Meaning |
| --- | --- | --- |
| `x0` | `zero` | hardwired to 0 (writes ignored) |
| `x1` | `ra` | return address |
| `x2` | `sp` | stack pointer |
| `x3` | `gp` | global pointer |
| `x4` | `tp` | thread pointer |
| `x5`–`x7` | `t0`–`t2` | temporaries (not preserved) |
| `x8` | `s0`/`fp` | saved register / frame pointer |
| `x9` | `s1` | saved register |
| `x10`–`x17` | `a0`–`a7` | function arguments / return values |
| `x18`–`x27` | `s2`–`s11` | saved registers (preserved) |
| `x28`–`x31` | `t3`–`t6` | temporaries (not preserved) |

RISC-V is a **load/store** architecture: arithmetic operates only on
registers; memory is accessed with dedicated load/store instructions. With
the `F`/`D` extensions there are 32 floating-point registers `f0`–`f31`.

## Calling convention

Arguments are passed in `a0`–`a7` (the rest on the stack); return values come
back in `a0` (and `a1` for 64-bit/double). The caller may clobber `t*` and
`a*`; the callee must preserve `s*` and `sp`. `ra` holds the return address;
`jal`/`jalr` set it, `ret` jumps through it.

## CSRs

Control and status registers (CSRs) configure the core: interrupt enables,
exception cause, privilege level, and counters. They are accessed with the
dedicated `csrr`/`csrw`/`csrs`/`csrc` instructions, not normal loads/stores.

Key CSRs on this core:

| CSR | Meaning |
| --- | --- |
| `mstatus` / `sstatus` | global status (interrupt enables, privilege) |
| `mie` / `mip` | machine interrupt enable / pending |
| `mcause` / `scause` | last exception/interrupt cause |
| `mtvec` / `stvec` | trap handler base address |
| `mepc` / `sepc` | program counter to return to after a trap |
| `mcycle` / `minstret` | cycle / instruction counters |

`mcycle` is used in the board notes to measure the actual core clock (the
measured ~189 MHz value).

## Privilege modes

RISC-V has three privilege levels, each with its own CSR set:

| Mode | Name | Typical use |
| --- | --- | --- |
| M | Machine | highest; bare-metal, boot, firmware |
| S | Supervisor | OS kernel (Embox runs here) |
| U | User | application code |

Lower modes cannot access higher-mode CSRs; privileged instructions trap.
The K1921VG1T supports M, S, and U, so Embox (and Linux) run in S-mode with an
MMU. See [Core](../riscv/k1921vg1t/core.md) for the exception/interrupt codes
and priorities.

## Common instructions

| Instruction | Effect |
| --- | --- |
| `lw` / `sw` | load/store word (4 bytes) |
| `lh`/`lhu` / `sh` | load/store halfword |
| `lb`/`lbu` / `sb` | load/store byte |
| `add`, `sub`, `and`, `or`, `xor`, `sll`, `srl`, `sra` | integer arithmetic/logic |
| `addi`, `andi`, `ori`, `xori`, `slli`, `srli`, `srai` | immediate forms |
| `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu` | conditional branches |
| `jal`, `jalr` | jump / jump-and-link |
| `lui`, `auipc` | load upper immediate (build addresses) |
| `ecall` | environment call (system call / trap) |
| `ebreak` | breakpoint |
| `fence.i` | instruction-fetch fence (self-modifying code) |

The `C` (compressed) extension adds 16-bit encodings of common instructions
to shrink code size; the disassembler shows them as their 32-bit equivalents.

## Sources

- [RISC-V unprivileged specification](https://riscv.org/technical/specifications/)
- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 12
