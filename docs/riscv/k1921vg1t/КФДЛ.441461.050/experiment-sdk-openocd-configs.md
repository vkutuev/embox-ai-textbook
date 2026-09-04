# Experiment: SDK OpenOCD Configurations

Date: 2026-09-04. Board rebooted (power cycle), Embox image in flash,
chip starts in LOCKSTEP mode (CFGWORD default) and exposes a single debug
hart.

Tested the stock OpenOCD configs from
[k1921vg1t_sdk](https://gitflic.ru/project/niiet/k1921vg1t_sdk)
(`tools/openocd/`) with the NIIET OpenOCD build
(`0.12.0+dev-g3c9c5dc`) and a J-Link V9.

## Test matrix

| # | Config | Result |
| - | ------ | ------ |
| 1 | `openocd-snippets/k1921vg1t/connect_jlink_jtag.cfg` (sources `target/k1921vg1t.cfg`, dualcore) | **FAILS**: `cpu0` examination "unavailable", `cpu1` "Hart index 1 is too large. The maximum index for this Debug Module is 0", then `reset init` times out |
| 2 | `target/k1921vg1t_singlecore.cfg` + `interface/jlink.cfg` | **WORKS**: single tap, examination OK (`XLEN=32, misa=0x4014912d`), halt/reg/mdw fine |
| 3 | `board/niiet-dev-k1921vg1t.cfg` + `interface/jlink.cfg` | **FAILS**: dualcore examination error, plus script error `can't read "_TARGETNAME"` (board cfg expects `_TARGETNAME`, dualcore cfg defines only `_TARGETNAME_0/_1`) |
| 4 | `target/k1921vg1t_dualcore.cfg` | equivalent to test 1 (fails the same way) |

## Why the dualcore configs fail

In LOCKSTEP mode the two taps share one debug module with a single hart
(hart index max = 0). The dualcore configs create two targets with
`-coreid 0` and `-coreid 1` on two taps; the second hart does not exist
in this mode. The singlecore config with one tap and `-coreid 0` matches
the silicon.

The configured tap gets the live debug module; OpenOCD auto-discovers
the second tap and prints
`Warn : AUTO auto0.tap - use "jtag newtap auto0 tap -irlen 5 -expected-id 0xdeb15001"`
(a warning, not an error).

## Reset behavior

- **SRST is not wired** on this adapter/board combination:
  `adapter assert srst` leaves the UART CR register (`0x52010030`)
  unchanged, so the J-Link RESET pin does not reach the MCU reset.
- `reset init`/`reset halt` always time out:
  `Hart didn't leave reset in 5s; dmstatus=0x400382 (allhavereset=false)`.
- **`ndmreset` works**: writing `dmcontrol` (`dmi 0x10`) with bit 1 set
  resets the whole system from the debug module:

  ```
  > riscv dmi_write 0x10 0x80000003    # dmactive + ndmreset + haltreq
  > riscv dmi_write 0x10 0x80000001    # deassert ndmreset
  ```

  Afterwards the core is at the reset vector (pc = 0x00000000).

## UART corruption after hart-level resets

A hart-only reset or a soft restart (`reg pc 0x0` + `resume`) after the
chip had already booted once left the UART clock domain corrupted:
the console produced garbage at 115200 (wrong effective baud) and the
receive FIFO flooded with break/framing errors, even though all register
settings looked correct (RCU `UARTCFG0=0x111` = HSE 27 MHz, `CPE=0`,
pl011 `IBRD/FBRD` = 115200 @ 27 MHz). A full system reset via
`ndmreset` restores the console.

## Working bring-up sequence (no SRST)

```
source [find interface/jlink.cfg]
transport select jtag
source [find target/k1921vg1t_singlecore.cfg]

init
halt
riscv dmi_write 0x10 0x80000003
riscv dmi_write 0x10 0x80000001
reg pc 0x0
resume
```

Result: clean Embox boot log at 115200 on `/dev/ttyUSB0`, `hello_world`
prints `Hello, world!`.

## Conclusion

For this board state (LOCKSTEP after power-on), the only stock SDK
config that connects is `k1921vg1t_singlecore.cfg`. Use `ndmreset` for
system reset since SRST is not wired; always reset via `ndmreset`
before starting the flashed image to avoid the UART clock corruption.
