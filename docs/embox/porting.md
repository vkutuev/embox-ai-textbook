# Porting to a New Architecture

This page describes the minimal requirements for Embox to start on a new CPU
architecture. Assume the new architecture is called `newarch`; refer to the
existing architectures under `src/arch/` as examples.

## Architecture modules

1. Create the directory `src/arch/newarch/`.
2. Create `src/arch/newarch/embox.lds.S` describing the kernel memory map.
3. Create subdirectories as needed (usually at least `kernel/`). The module
   system is not mapped to the directory structure, but it is better to
   follow the structure of other architectures.
4. Create `src/arch/newarch/Mybuild` with stubs for the basic
   architecture-dependent modules:

```
package embox.arch.newarch

module arch extends embox.arch.arch {
    /* Implements arch_init(), arch_idle() and other functions */
}

module interrupt extends embox.arch.interrupt {
    /* Provides ipl_impl.h with ipl_init(), ipl_save(), ipl_restore() */
}

module locore extends embox.arch.locore {
    /* Platform-specific entry: setup registers, zero .bss,
     * copy .data, handle syscalls, etc. */
}

module mem_barriers extends embox.arch.mem_barriers {
}
```

## Drivers

A serial port driver is required to communicate with the system. Pick an
existing one from `src/drivers/serial/` if the target platform is supported,
or implement a new one. If no interrupt controller or timer drivers exist
for the platform, look at `src/drivers/interrupt/` and
`src/drivers/clock/`.

## Loader

The first thing to implement is the loader. Basic steps:

1. Set up the exception table base, turn off the MMU, and so on.
2. Zero the `.bss` section.
3. Copy `.data` from ROM to RAM if the image is stored in ROM.
4. Jump to `kernel_start`, the platform-independent part of the kernel start.

Context switch, exception handlers, and other platform-dependent parts can
initially be stubs.

## Template

- `mods.conf` — list of included modules
- `build.conf` — cross-compiler and build options:

```
TARGET = embox
ARCH = newarch
CROSS_COMPILE = newarch-      /* newarch-gcc, newarch-ld, ... */
CFLAGS += -O0 -g
LDFLAGS += -N -g
```

- `lds.conf` — memory sections (RAM/ROM, sizes, base addresses). Check the
  base address carefully: the system will not start if control is not passed
  to the expected address.

```
RAM (0x40000000, 256M)   /* 256 MiB starting at 0x4000_0000 */
ROM (0x50000000, 1024M)  /* 1 GiB starting at 0x5000_0000 */

text   (ROM)
rodata (ROM)
data   (RAM)
bss    (RAM)
```

## Sources

- [Embox wiki: Porting to a new architecture](https://github.com/embox/embox/wiki/Porting-to-a-new-architecture)
