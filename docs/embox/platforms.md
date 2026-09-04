# Supported Platforms

Embox supports several CPU architectures:

- ARM
- x86 (32-bit)
- SPARC
- DLX (Microblaze)
- MIPS
- PPC (PowerPC)
- E2k (Elbrus)
- RISC-V

Each architecture requires its own developer environment (cross-compiler and
GDB); see [Quick start](quick-start.md).

## QEMU

Embox supports the following QEMU machines:

- `qemu-system-i386`
- `qemu-system-arm`
- `qemu-system-mips`
- `qemu-system-ppc`
- `qemu-system-microblaze`
- `qemu-system-sparc`
- `qemu-system-riscv32` / `qemu-system-riscv64`

Configure with `make confload-<ARCH>/qemu` where `<ARCH>` is `x86`, `arm`,
`mips`, `ppc`, `microblaze`, `sparc`, or `riscv`, then run
`./scripts/qemu/auto_qemu`. The script parses the config files from `conf/`
and starts QEMU with the corresponding arguments.

## ARM

Embox supports most ARM sub-architectures: ARM7, ARM9, ARM Cortex-M, and
ARM Cortex-A. All of them use the `arm-none-eabi-gcc` cross-compiler.

Popular ARM-based hardware platforms:

- STM32 MCU eval tools and discovery boards
- Raspberry Pi
- BeagleBoard

## x86

Embox supports 32-bit x86 with some peripheral drivers. `gcc` is used as the
cross-compiler on Linux. Supported targets:

- `qemu-system-i386`
- Booting with GRUB
- BifferBoard

## RISC-V

Embox runs on QEMU RISC-V machines (`virt`, `sifive_e`, `sifive_u`). The
`riscv/qemu` template targets the QEMU `virt` machine with
`riscv64-unknown-elf-gcc` as the cross-compiler:

```console
$ make confload-riscv/qemu          # other templates: riscv/sifive_e, riscv/sifive_u
$ make
$ ./scripts/qemu/auto_qemu          # or: qemu-system-riscv32 -M virt -nographic -bios build/base/bin/embox
```

The `sifive_e` machine requires QEMU 4.1.0 or newer. See
[Quick start](quick-start.md) for cross-compiler installation and a verified
Hello World example.

## Sources

- [embox-docs: Supported platforms](https://github.com/embox/embox-docs/blob/master/en/platforms/embox_supported_platforms_en.md)
- [embox-docs: QEMU emulator support](https://github.com/embox/embox-docs/blob/master/en/platforms/embox_supported_platforms_qemu_en.md)
- [embox-docs: ARM](https://github.com/embox/embox-docs/blob/master/en/platforms/embox_supported_platforms_arm_en.md)
- [embox-docs: x86](https://github.com/embox/embox-docs/blob/master/en/platforms/embox_supported_platforms_x86_en.md)
- [Embox wiki: QEMU RISC-V emulator](https://github.com/embox/embox/wiki/QEMU-RISC-V-emulator)
