# Quick Start

The best way to get started with Embox is to run it on the QEMU emulator,
which supports different CPU architectures.

## Getting Embox

```console
$ git clone https://github.com/embox/embox
```

or download an archive from
[github.com/embox/embox/releases](https://github.com/embox/embox/releases).

## Host environment

### Linux

Minimal required packages: `make`, `gcc`, and a cross-compiler for the target
platform. Optional but recommended: `build-essential`, `gcc-multilib`, `curl`,
`libmpc-dev`, `python`.

```console
# Debian/Ubuntu
$ sudo apt install make gcc build-essential gcc-multilib curl libmpc-dev python

# Arch
$ sudo pacman -S make gcc-multilib cpio qemu

# Fedora
$ sudo dnf install make gcc cpio qemu patch curl python
```

### Windows

Use WSL2 with a Linux distribution installed from the Microsoft Store
(Embox does not run under WSL1).

```console
$ wsl -l -v
```

### macOS

```console
$ brew install i686-elf-binutils i686-elf-gcc awk cpio qemu python
```

Note: on macOS the system `gcc` may be a symlink to `clang`, which is not
supported. In this case set `CROSS_COMPILE = i686-elf-` in `conf/build.conf`.

## Cross-compilers

- **x86**: `sudo apt install gcc` plus `gcc-multilib`.
- **ARM**: `sudo apt install arm-none-eabi-gcc` (Ubuntu) or
  `sudo apt install gcc-arm-none-eabi` (Debian). Alternatively, download the
  archive from [launchpad.net/gcc-arm-embedded](https://launchpad.net/gcc-arm-embedded)
  and add its `bin` directory to `PATH`.
- **RISC-V**: use `riscv64-unknown-elf-gcc`. Options: build the
  [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)
  from source (configure with multilib), download a prebuilt
  `riscv64-unknown-elf-toolchain.tar.bz2` from
  [github.com/embox/crosstool/releases](https://github.com/embox/crosstool/releases)
  (verified to work with Embox), or use the SiFive prebuilt toolchains from
  [sifive.com/software](https://www.sifive.com/software). On macOS, install
  via [homebrew-riscv](https://github.com/riscv-software-src/homebrew-riscv).
- **SPARC, Microblaze, MIPS, PowerPC, MSP430**: use the
  [embox/crosstool](https://github.com/embox/crosstool) project — either
  ready-to-use archives from
  [github.com/embox/crosstool/releases](https://github.com/embox/crosstool/releases)
  or build your own with `./crosstool.sh ARCH` (produces an
  `ARCH-elf-toolchain.tar.bz2` archive).

## QEMU installation

Supported CPU architectures: `x86`, `ARM`, `MIPS`, `Sparc`, `PPC`,
`Microblaze`, `RISC-V`.

```console
$ sudo apt install qemu-system-<ARCH>   # <ARCH>: i386, arm, sparc, mips, ppc, misc, riscv
```

or install everything at once with `sudo apt install qemu-system`.

For RISC-V, `qemu-system-riscv32`/`qemu-system-riscv64` are needed; the
`sifive_e` machine is supported since QEMU 4.1.0.

## Build and run on QEMU

```console
$ make confload-<ARCH>/qemu   # <ARCH>: x86, arm, mips, ppc, sparc, microblaze, riscv
$ make                        # or: make -j4
$ ./scripts/qemu/auto_qemu
```

For RISC-V the `riscv/qemu` template targets the QEMU `virt` machine, and the
built image is passed to QEMU with `-bios`:

```console
$ make confload-riscv/qemu
$ make
$ ./scripts/qemu/auto_qemu    # or: qemu-system-riscv32 -M virt -nographic -bios build/base/bin/embox
```

`auto_qemu` creates a TUN/TAP interface and needs superuser rights and the
`ethtool` utility (`sudo apt install ethtool`). Network connection can be
checked by pinging `10.0.2.16`; a terminal is available via `telnet`.

After boot, a command prompt appears. Type `help` to list available commands.
Press `Ctrl+A` then `X` to exit QEMU.

## Hello world application

An Embox application is a regular Embox module with special attributes. Its
source code is an ordinary POSIX program written in C and can be compiled
under Linux too.

1. Create `src/cmds/hello_world/hello_world.c`:

   ```c
   #include <stdio.h>

   int main(int argc, char **argv) {
       printf("Hello, world!\n");
       return 0;
   }
   ```

2. Create `src/cmds/hello_world/Mybuild`:

   ```
   package embox.cmd

   @AutoCmd
   @Cmd(name = "hello_world", help = "First Embox application")
   module hello_world {
       source "hello_world.c"
   }
   ```

3. Add the module to `conf/mods.conf`:

   ```
   include embox.cmd.hello_world
   ```

4. Rebuild and run:

   ```console
   $ make
   $ ./scripts/qemu/auto_qemu
   ```

The command `hello_world` now appears in the output of `help` and prints:

```console
root@embox:/#hello_world
Hello, world!
```

This example was verified on QEMU RISC-V (`make confload-riscv/qemu` with the
`riscv64-unknown-elf-gcc` toolchain).

## Debugging

```console
$ sudo ./scripts/qemu/auto_qemu -s -S -no-kvm
```

QEMU then waits for a GDB client. In another terminal:

```console
$ gdb ./build/base/bin/embox
(gdb) target extended-remote :1234
(gdb) continue
```

At any moment press `Ctrl+C` in GDB to inspect the current thread stack with
`backtrace` or set breakpoints (`break <function name>`,
`break <file name>:<line number>`).

## Sources

- [embox-docs: Quick start](https://github.com/embox/embox-docs/blob/master/en/embox_quick_start_en.md)
- [embox-docs: Running and debugging](https://github.com/embox/embox-docs/blob/master/en/embox_execute_debug_en.md)
- [Embox wiki: Quick Start](https://github.com/embox/embox/wiki/Quick-Start)
- [Embox wiki: QEMU RISC-V emulator](https://github.com/embox/embox/wiki/QEMU-RISC-V-emulator)
