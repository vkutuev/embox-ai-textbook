# Cross-compilation

## Native vs. cross-compilation

A **compiler** turns source code into machine code. When the machine that runs
the compiler is the same kind of machine that runs the result, this is
**native compilation** (e.g. compiling a Linux program on Linux).

**Cross-compilation** produces machine code for a *different* architecture
than the one the compiler runs on. To build Embox for the K1921VG1T you run
the compiler on an x86-64 Linux host, but the output is RISC-V machine code
that only the microcontroller can execute.

## Toolchain triple

A cross-toolchain is identified by a **target triple**, conventionally
`<arch>-<vendor>-<os>-<abi>`. The RISC-V toolchain used for Embox on the
K1921VG1T is:

```
riscv64-unknown-elf
```

- `riscv64` — the compiler defaults to the RV64 base ISA, but the `-march`
  flag (see below) selects the RV32 ISA actually implemented by the chip.
- `unknown` — no specific vendor.
- `elf` — bare-metal target: no operating system, output is a raw ELF binary.

The toolchain provides `riscv64-unknown-elf-gcc` (C compiler), `-as`
(assembler), `-ld` (linker), `-objcopy`, `-objdump`, and `-gdb`.

## The `-march` and `-mabi` flags

`-march` selects the instruction set; `-mabi` selects the calling convention.
The K1921VG1T core implements RV32IMAFDCP (with Zicsr/Zifencei). The vendor
recommends:

```
-march=rv32imafdcp_zicsr -mabi=ilp32d
```

The Embox template uses a reduced subset sufficient for Embox:

```
-march=rv32imfdc_zicsr_zifencei -mabi=ilp32d
```

Letters: `I` integer base, `M` multiply/divide, `A` atomics, `F`/`D`
single/double float, `C` compressed instructions, `P` DSP/SIMD, `Zicsr` CSR
instructions, `Zifencei` instruction-fetch fence. `ilp32d` means 32-bit
`int`/`long`/pointers with double-precision floats passed in FP registers.

## Toolchain options for this board

Any of the following produces a working `riscv64-unknown-elf` toolchain:

1. **Syntacore Development Toolkit (SC-DT)** — a ready-to-use SDK from
   Syntacore (<https://syntacore.com/tools/development-tools>); combine it
   with the NIIET OpenOCD patches (see [References](../references.md)).
2. **Embox prebuilt toolchain** — download the archive from
   <https://github.com/embox/crosstool/releases/tag/2.44-14.3.0-15.2> and add
   its `bin` directory to `PATH`.
3. **OS package manager** — install a `gcc-riscv64-unknown-elf` or
   `riscv-none-elf-gcc` package (name varies by distribution).
4. **Build from source** — via CrosstoolNG, Buildroot, or the
   [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)
   (configure with `--enable-multilib` for `rv32imfdc`).

After installing, verify with:

```console
$ riscv64-unknown-elf-gcc --version
$ riscv64-unknown-elf-gcc -print-multi-lib
```

The second command lists the `-march`/`-mabi` combinations the toolchain can
actually target; make sure `rv32imfdc` (or `rv32imafdc`) is present.

## Why the host can't run the result

A cross-compiled Embox image is RISC-V ELF. You cannot execute it on an x86-64
host directly; you either flash it to the microcontroller or run it in the
QEMU RISC-V emulator (see [Embox Quick start](../embox/quick-start.md)).

## Sources

- [RISC-V GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)
- [Embox crosstool releases](https://github.com/embox/crosstool/releases)
- [Syntacore Development Tools](https://syntacore.com/tools/development-tools)
