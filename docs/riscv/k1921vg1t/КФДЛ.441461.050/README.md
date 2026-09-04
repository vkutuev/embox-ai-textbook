# КФДЛ.441461.050 (NIIET-DEV-K1921VG1T)

Development board for the K1921VG1T microcontroller, verified with a real
"Hello World" run of Embox.

## Bring-up checklist

| Item | Detail |
| ---- | ------ |
| JTAG adapter | J-Link V9 (TCK 100 kHz; the SDK limit for K1921VG1T is 200-250 kHz) |
| Console | UART0 (PB1 = TX, PB0 = RX, alt function 3), 115200 baud, `/dev/ttyUSB0` |
| Embox template | `platform/niiet/k1921vg1t_dev` (`make confload-platform/niiet/k1921vg1t_dev`) |
| Flash layout | text/rodata in ROM `0x00000000` (4 MiB), data/bss in RAM `0x20000000` (1 MiB) |

## Building Embox

```console
$ export PATH=<riscv64-unknown-elf-toolchain>/bin:$PATH
$ make confload-platform/niiet/k1921vg1t_dev
$ echo "include embox.cmd.hello_world" >> conf/mods.conf   # inside the configuration block
$ make -j16
```

The image is `build/base/bin/embox`. The template uses
`-march=rv32imfdc_zicsr_zifencei -mabi=ilp32d` (the P extension is not
needed), PLIC at `0xFE000000`, timer TMR0 at `0x51008000`, console on
`pl011` at `0x52010000`.

## Building OpenOCD

Use the NIIET fork with the K1921VG1T flash driver:

```console
$ git clone https://gitflic.ru/project/niiet/openocd.git
$ cd openocd && ./bootstrap
$ ./configure --enable-jlink --disable-werror && make -j16
```

Configs come from the per-MCU SDK:

```console
$ git clone https://gitflic.ru/project/niiet/k1921vg1t_sdk.git
```

Relevant files: `tools/openocd/scripts/interface/jlink.cfg`,
`tools/openocd/scripts/target/k1921vg1t{,_singlecore,_dualcore}.cfg`,
`tools/openocd/openocd-snippets/k1921vg1t/connect_jlink_jtag.cfg`.

## Board quirks discovered during bring-up

- **Lockstep mode.** After power-on the chip starts in LOCKSTEP mode
  (CFGWORD default), so the debug module of tap `cpu1` exposes a single
  hart (`coreid 0`). The dual-core config from the SDK
  (`k1921vg1t.cfg`) fails to examine both targets; a single target on tap
  `cpu1` with `-coreid 0` works.
- **No hardware reset via J-Link.** `reset init` times out
  ("Hart didn't leave reset in 5s; dmstatus=0x400382"); SRST is not wired
  to the adapter. Workaround: flash with the core halted and start Embox
  with `reg pc 0x0` + `resume`.
- The `k1921vg1t` flash driver displays CFGWORD write protection as
  "enable", but register-based erase/write still works.

## Flashing in dualcore mode (SRV-ON jumper)

The dualcore OpenOCD configs from the SDK (`k1921vg1t.cfg`,
`k1921vg1t_dualcore.cfg`) only work when the chip is in DUALCORE mode.
After power-on the chip starts in LOCKSTEP mode (default CFGWORD,
`LOCKSTEPEN=1`), where the two cores form a single logical hart — the
dualcore config fails with `Hart index 1 is too large` / `unavailable`.

The SRV-ON jumper pulls the `SERVEN` pin high. When the board is reset
with the jumper on, the chip enters **service mode** (UM chapter 9.4):

- Flash reads return zeros, so the configuration word (CFGWORD at
  `0x003F_FFF0`) reads as `0x00000000` → `LOCKSTEPEN=0` → the cores start
  in **DUALCORE mode** and each exposes its own debug hart.
- The `JTAGEN` bit is ignored, so debug is always available.
- Flash write protection (`FLASHWE`) reads as disabled; in service mode
  the flash is locked for reading/execution and only the full erase
  (`k1921vg1t srv_erase` via `SIU.SERVCTL`) is allowed — after the erase
  the flash controller accepts programming through the register
  interface.

Procedure:

1. Ask the user to place the SRV-ON jumper and power-cycle the board.
2. Connect OpenOCD with the SDK dualcore config
   (`openocd-snippets/k1921vg1t/connect_jlink_jtag.cfg`).
3. Run `k1921vg1t srv_erase` (full erase, incl. NVR/CFGWORD), then flash
   the image (`flash write_image ... verify`).
4. Ask the user to remove the jumper and power-cycle again. With the
   jumper off, the chip reads the real CFGWORD (all-ones after erase →
   `LOCKSTEPEN=1`) and starts in LOCKSTEP mode again — Embox runs fine
   in both modes.

> **Agent instruction**: when flashing this board with a dualcore config,
> always ask the user to place the SRV-ON jumper and power-cycle the
> board first, and to remove the jumper and power-cycle again after
> flashing.

## Flashing and running

OpenOCD config (lockstep, J-Link):

```
source [find interface/jlink.cfg]
transport select jtag
adapter speed 100
reset_config trst_only

jtag newtap K1921VG1T cpu1 -irlen 5 -expected-id 0xDEB15001
jtag newtap K1921VG1T cpu0 -irlen 5 -expected-id 0xDEB15001

target create K1921VG1T.cpu riscv -endian little -chain-position K1921VG1T.cpu1 -coreid 0 -rtos hwthread
K1921VG1T.cpu configure -work-area-phys 0x20000000 -work-area-size 0x8000 -work-area-backup 0

flash bank K1921VG1T.flash k1921vg1t 0x0 0 0 16 K1921VG1T.cpu

init
halt
flash probe 0
```

```console
$ openocd -s k1921vg1t_sdk/tools/openocd/scripts -f connect.cfg
$ telnet localhost 4444
> flash erase_sector 0 0 last
> flash write_image /path/to/embox
> verify_image /path/to/embox
> reg pc 0x0
> resume
```

Console output on `/dev/ttyUSB0` (115200 8N1):

```
[info] (Kernel) Embox kernel start
...
Started shell [tish] on device [ttyS0]
root@embox:/#hello_world
Hello, world!
```

Verified on a real board (J-Link V9, UART over CH340 at `/dev/ttyUSB0`).

## Sources

- Embox source: <https://github.com/embox/embox> (template
  `platform/niiet/templates/k1921vg1t_dev`)
- OpenOCD fork: <https://gitflic.ru/project/niiet/openocd>
- SDK: <https://gitflic.ru/project/niiet/k1921vg1t_sdk>
- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (chapters 9, 13, 41)
