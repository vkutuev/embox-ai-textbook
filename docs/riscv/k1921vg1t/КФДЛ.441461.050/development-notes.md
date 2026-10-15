# Development notes

Consolidated, cross-cutting knowledge from the example bring-up sessions
on the КФДЛ.441461.050 board. Hardware-specific facts (J-Link wiring,
reset, console) live in [README.md](README.md); the per-example findings
live in each `experiment-*.md`. This file captures the reusable patterns,
recurring gotchas, and open questions that span multiple examples.

## 1. Adding an example — the pattern

Every example follows the same shape (see `examples/niet-dev-k1921vg1t/`):

```
examples/niet-dev-k1921vg1t/<name>/
    <name>.c          # the command (or driver) source
    Mybuild           # Embox module definition
    mods.conf.snippet # `include` lines to paste into conf/mods.conf
    README.md         # wiring + install + run
```

**A command** (`Mybuild`):

```mybuild
package embox.cmd.example.<name>

@AutoCmd
@Cmd(name = "<name>", help = "...", man = '''...''')
module <name> {
	source "<name>.c"
	depends <core module exporting the header it includes>
}
```

The `depends` names the module whose `@IncludeExport` header the source
includes. Observed mappings: GPIO → `embox.driver.gpio.core`, I2C →
`embox.driver.i2c_core`, PWM → `embox.driver.pwm.pwm_dev`, DMA →
`embox.driver.dma.dma_new`, RTC → `embox.driver.rtc.rtc`.

**A driver** (only the RTC so far): source goes into
`src/drivers/<subsys>/niiet/`, `Mybuild` has
`package embox.driver.<subsys>.niiet`, the source implements the subsystem
`*_ops` struct and registers with the `*_DEVICE_DEF` macro, and the init
is an `EMBOX_UNIT_INIT`.

**Build loop** (repeatable):

```console
$ cp -r examples/niet-dev-k1921vg1t/<name>/ .tmp/embox/src/cmds/example/
$ cd .tmp/embox
$ sed -i -e '$d' conf/mods.conf          # drop the closing '}'
$ printf '\tinclude embox.cmd.example.<name>.<name>\n}\n' >> conf/mods.conf
$ make -j16
```

## 2. Header and libc gotchas

- `dma_new.h` is exported as `drivers/dma.h` (via `target_name="dma.h"`),
  so include `<drivers/dma.h>`, **not** `<drivers/dma/dma_new.h>`.
- `mktime`/`gmtime_r`/`strptime` live in `embox.compat.posix.time.time`;
  pull it in when a command or driver needs calendar time.
- Embox `printf` float support is an option (`support_floating`); the ADC
  examples avoided `%f` by computing in fixed point.

## 3. Recurring software gotchas

- **IRQ re-attach leak.** `irq_attach()` leaks across command re-runs: a
  second run of the same command fails with "cannot attach". Call
  `irq_detach(irq_nr, dev_id)` before re-attaching (same pattern in GPIO,
  timer, and DMA examples). Handlers return `IRQ_HANDLED` (1), not 0.
- **Register RMW discipline.** A "pulse" write must preserve the other
  bits. The ADC `SEQSYNC` trigger must be written as `SYNC0 | GSYNC` —
  writing only `GSYNC` clears `SYNC0` and silently disables the trigger.
- **Save/restore across reset.** `RCU.RTCCFG.RSTDIS` clears the RTC
  counter, so the RTC init must save `TIME`, reset, and restore it (the
  SDK does this; omitting it reset the clock to 1970 every boot).
- **Static globals persist across command re-runs** — reset per-run state
  in `main()`.
- **Probe the device, not the MCU pins.** During the HC-SR04 debug, the
  analyzer on the board's D0/D1 pins showed the MCU state, not the sensor;
  move the probes to the sensor side to see what it actually does.

## 4. Console / debug environment

- The CH340 console **corrupts long output** at 115200 (multi-line dumps
  are garbled; short command output and single-byte `i2cget` are
  reliable). Don't trust a garbled `i2cdump` grid — re-read single bytes.
- A **GDB attach halts the target**; an SRST pulse (or `ndmreset`)
  recovers it. After GDB, the target does not auto-resume.
- Always do a **system reset** before starting a freshly flashed image
  (see README.md — hart-only resets leave the UART clock domain broken).

## 5. Clock discrepancies (open)

Several sources disagree; reconcile with one controlled measurement
(`mcycle` vs TMR `COUNT` vs a real output) before trusting any one:

| Source | Clock |
| ------ | ----- |
| `mcycle` / TMR counter (timer example) | ~189 MHz (27 MHz HSE × 7) |
| PWM (TMR in PWM mode, measured 1.0 kHz) | 150 MHz (= `SYSTEM_FREQ`) |
| RCU PLL config intent (driver comment) | 60 MHz |
| I2C driver `I2C_SYSTEM_FREQ` | hardcoded 50 MHz |
| UART `pl011` `uartclk` | 27 MHz (HSE), shared across UARTs |

The PWM measurement (requested 1000 Hz → exactly 1000 Hz) is the cleanest
peripheral-clock data point so far.

## 6. Embox driver bugs / quibbles found (upstream-worthy)

- `niiet_dma` IRQ handler returns `0` instead of `IRQ_HANDLED` (works, but
  wrong).
- Stock `pwm` command's `-d` passes the channel number rather than
  `1 << chan` to `pwm_enable()` (enables the wrong channel).
- `niiet_gpio` IRQ attach bookkeeping (patch
  `0001-niiet_gpio-irq-attach-bookkeeping.patch` in the LED example).
- `vg015_i2c` RX path logs "expected MRADPA, got 0x21" when `i2cdetect -r`
  trips reserved-address HS mode.
- `pl011` uses one shared `uartclk` option for all UARTs (fine when they
  share HSE, but a trap if they don't).

## 7. Embox driver availability for the K1921VG1T

- **Has niiet drivers**: GPIO, serial (pl011), I2C (vg015), PWM, DMA,
  SPI (pl022), CAN, flash (mflash), RTC (added here), clk/clock,
  sys_ctrl, USB gadget.
- **Framework exists, driver added here**: RTC.
- **No driver — programmed directly in examples**: ADC (`tsensor`,
  `sharp`), SDRAM/EMC (`sdram`).
- **No driver, documented but no example yet**: CRC, HASH, TRNG, WDT, DAC,
  I2S, Ethernet (register maps are in `docs/riscv/k1921vg1t/`).

## 8. Remaining candidate examples

- **Motor quadrature encoder** (ASLONG JGA25-370) — GPIO-IRQ quadrature
  decode, hand-cranked; uses hardware already owned.
- **SPI loopback** — one jumper wire (MOSI→MISO) through `pl022_spi`.
- **CRC/HASH or TRNG** — zero hardware, needs a baremetal driver.
- **WDT** — watchdog reset demo (zero hardware, new driver).
- **DAC** — analog output on the board's SMA (needs a scope/DMM to verify).
- **DMA + peripheral** (UART/SPI) and **PWM + DMA sine** — build on DMA.
- **HC-SR04** — needs two resistors (5 V + divider); the code exists.
- **CAN** — needs a transceiver (check loopback mode first).
