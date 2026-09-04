# Experiment: Flashing via GDB `load`

Date: 2026-09-04. Board rebooted, chip in LOCKSTEP mode, J-Link V9,
console on `/dev/ttyUSB0` (115200). GDB: system
`riscv64-linux-gnu-gdb` (GDB 17.2). OpenOCD: NIIET build
`0.12.0+dev-g3c9c5dc` with the SDK `k1921vg1t_singlecore.cfg` config.

## Setup

OpenOCD with overridden GDB flash events (see below):

```
source [find interface/jlink.cfg]
transport select jtag
source [find target/k1921vg1t_singlecore.cfg]
K1921VG1T.cpu configure -event gdb-flash-erase-start { halt }
K1921VG1T.cpu configure -event gdb-flash-write-end { halt }
init
halt
```

GDB command file:

```
set pagination off
set confirm off
target extended-remote :3333
monitor halt
load
compare-sections
monitor riscv dmi_write 0x10 0x80000003
monitor riscv dmi_write 0x10 0x80000001
set $pc = 0x0
continue
```

Run:

```console
$ openocd -s k1921vg1t_sdk/tools/openocd/scripts -f gdbflash.cfg &
$ riscv64-linux-gnu-gdb -x gdb_load.cmd build/base/bin/embox
```

## Results

`load` programmed the image through the OpenOCD flash driver:

```
Loading section .text, size 0x49b3c lma 0x0
Loading section .rodata, size 0x1a020 lma 0x49b40
Loading section .data, size 0xe2c lma 0x63b60
Start address 0x00000000, load size 412040
Transfer rate: 5 KB/sec, 14715 bytes/write.
```

`compare-sections` read the flash back and all sections matched:

```
Section .text, range 0x0 -- 0x49b3c: matched.
Section .rodata, range 0x49b40 -- 0x63b60: matched.
Section .data, range 0x63b60 -- 0x6498c: matched.
```

After an `ndmreset` + `reg pc 0x0` + `resume` (SRST is not wired, see the
SDK configs experiment), Embox booted from the GDB-flashed image and
`hello_world` printed `Hello, world!` on the console.

## Gotchas

1. **Default `gdb-flash-erase-start` event breaks the flow.** The default
   handler runs `reset init`, which times out on this chip
   (`Hart didn't leave reset in 5s`), leaving the target running and
   failing the erase with `flash_erase returned -304`:
   `Error erasing flash with vFlashErase packet`. Override the
   `gdb-flash-erase-start` and `gdb-flash-write-end` events with `halt`.
2. **Monitor commands fail while the target is running.** After
   `load`/`compare-sections` OpenOCD leaves the target running; the
   `monitor` ndmreset commands from the same GDB batch then fail with
   `Cannot execute this command while the target is running`. Reset and
   resume via the OpenOCD telnet port (or halt first from GDB).
3. **System GDB is fine.** The crosstool-built
   `riscv64-unknown-elf-gdb` could not run on this host (missing
   `libncurses.so.5`); the distro `riscv64-linux-gnu-gdb` (GDB 17.2)
   works for `load`/`compare-sections`/`continue`.
4. GDB `load` throughput was ~5 KiB/s (JTAG at 100 kHz) — 412 KiB took
   about 80 seconds.
