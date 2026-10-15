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

## Board layout (rev 1.1)

From the official board documentation (see Sources):

- **JTAG** — connector XP7: pin 2 = TMS, 4 = TCK, 6 = TDO, 8 = TDI,
  10 = **nRST** (RST# of the MCU), odd pins 1/3/5/7/9 = +3V3/GND/NC.
  **TRST** is on a separate connector XP8.
- **J-Link wiring (verified).** Board nRST (XP7 pin 10) → J-Link
  **RESET** (pin 15 of the standard 20-pin connector), XP7 +3V3 → J-Link
  VTref (pin 1), XP8 (TRST) → J-Link nTRST (pin 3). Wiring nRST to
  nTRST makes `adapter assert srst` a no-op and sends OpenOCD's TRST
  pulses into the MCU RST#. Use
  `reset_config srst_only srst_open_drain connect_deassert_srst`.
  Do **not** use `trst_and_srst`: with TRST wired to XP8, TRST pulses
  reset the debug module and desynchronize OpenOCD (DMI timeouts after
  `init`).
- **Service mode** — jumper between `SRV` (18) and `ON` (17) on XP3
  pulls SERVEN high (see "Flashing in dualcore mode" below).
- **Reset** — `RESET` button (SB2), `RST#` on XP3 pin 39, or `DTR#` of
  the CH340B with the `nRST ON` jumper (XP4) for the bootloader.
- **Console** — UART0 of the MCU (PB1 = TX, PB0 = RX, alt function 3) is
  wired to the CH340B USB-UART (connector XS4) at 115200 baud.
- **Crystals** — 27 MHz (BQ2, HSE), 25 MHz (BQ1, Ethernet), 32.768 kHz
  (BQ3, RTC).
- **SDRAM** — two SDRAM chips, 32 MB total.

## Board quirks discovered during bring-up

- **Lockstep mode.** After power-on the chip starts in LOCKSTEP mode
  (CFGWORD default), so the two cores form a single logical hart. The
  dualcore config from the SDK (`k1921vg1t.cfg`) fails to examine both
  targets; the singlecore config or one target on tap `cpu1` with
  `-coreid 0` works. IDCODE of both taps is `0xDEB15001`, IR length 5.
- **Hardware reset works, but `reset init` still fails.** With the
  corrected wiring, `adapter assert srst` really resets the MCU (the
  debug module goes dead during the pulse and Embox reboots from flash).
  However, OpenOCD's `reset init`/`reset halt` still time out
  ("Hart didn't leave reset in 5s; dmstatus=0x400382") — the debug
  module never reports `havereset` after SRST. Use a manual pulse
  instead: `adapter assert srst; sleep 300; adapter deassert srst`
  (verified: clean Embox boot and `hello_world`). Alternative: system
  reset via the debug module (ndmreset) —
  `riscv dmi_write 0x10 0x80000003` then
  `riscv dmi_write 0x10 0x80000001`.
- **Host serial baud.** The CH340B node re-enumerates after reconnecting
  and comes up at a default baud; garbage at 115200 capture means the
  host side: run `stty -F /dev/ttyUSB0 115200 raw -echo` first.
- **UART corruption after hart-only resets/soft restarts.** A hart reset
  or `reg pc 0x0` + `resume` on an already-booted chip left the console
  outputting garbage at 115200 (broken UART clock domain) while the
  kernel kept running. A full system reset (SRST pulse or `ndmreset`)
  restores the console. Always do a system reset before starting a
  freshly flashed image.
- The `k1921vg1t` flash driver displays CFGWORD write protection as
  "enable", but register-based erase/write still works.
- **RTC battery needs the XP6 jumper.** The CR1220 in the holder (X6) only
  backs up the RTC's `VBAT` if a jumper is installed on **XP6 ("BAT ON")**.
  Without it, `VBAT` is routed to `V_BAT` on XP3 pin 37 (external battery
  input). XP6 also lets you measure `VBAT` current (remove the jumper and
  insert an ammeter).
- **Measured core/timer frequency is ~189 MHz (27 MHz HSE × 7).** The
  RCU driver's PLL configuration (NDIV=100, RDIV=2, DIV1A=4, DIV1B=2)
  is *intended* for 60 MHz per its own comment, but the actual clock,
  measured via the `mcycle` CSR and the TMR counter (and confirmed by a
  timer blink rate), is ~189 MHz — a discrepancy to investigate. The
  board config declares `core_freq=150000000`, which also does not
  match. UART0 is clocked from HSE 27 MHz, so the console baud is
  unaffected.

## Debug and flashing guide

| Task | Approach |
| ---- | -------- |
| Flash in LOCKSTEP (default) | `target/k1921vg1t_singlecore.cfg` + `interface/jlink.cfg`, or a one-tap cfg (see below); halt, erase, `flash write_image`, `verify_image` |
| Flash in DUALCORE | SRV-ON jumper + reset, then SDK dualcore config (see below) |
| Hardware reset | `adapter assert srst; sleep 300; adapter deassert srst` (verified; `reset init` is broken on this chip) or `ndmreset` from the debug module |
| Flash via GDB | override `gdb-flash-erase-start`/`gdb-flash-write-end` with `halt`, then `load` + `compare-sections` |
| Flash without a debug adapter | one-time install of the UART bootloader ("Flasher") via JTAG, then `k1921vx_flasher` over UART (BOOTEN=A8, UART0 B1/B0, app at `0x2000`) — see [flash.md](../flash.md) |
| Run flashed image | SRST pulse (see above), or `riscv dmi_write 0x10 0x80000003` / `0x80000001`, `reg pc 0x0`, `resume` |

Detailed experiment records:

- [SDK OpenOCD configurations](experiment-sdk-openocd-configs.md) —
  config test matrix, reset behavior, UART corruption analysis
- [GDB `load` flash](experiment-gdb-flash.md) — GDB flow, event
  overrides, throughput, toolchain notes

Diagnostic helpers:

- [Debug checklist](debug-checklist.md) — ordered isolation steps for
  console/reset problems
- [tools/uart_capture.sh](tools/uart_capture.sh) — serial capture with
  explicit baud
- [tools/dump_state.cfg](tools/dump_state.cfg) — OpenOCD register dump
  for state inspection

## Examples

Cross-cutting patterns, gotchas, and open questions are consolidated in
[Development notes](development-notes.md).

- [LED/button counter](example-led-button-counter.md) — USER button
  increments a counter shown in binary on the four LEDs (board LED1–LED4);
  three variants (polling, GPIO interrupt, input subsystem), sources in
  [examples/niet-dev-k1921vg1t/led_button_counter/](../../../../examples/niet-dev-k1921vg1t/led_button_counter/).
  Polling and IRQ variants are verified on hardware; the input variant
  hits a GPIO interrupt quirk — see the
  [experiment journal](experiment-led-button-counter.md).
- [Timer blink](example-timer-blink.md) — LED1 blinking via a busy-wait
  loop vs. the TMR1 hardware timer interrupt (exact 1 Hz), sources in
  [examples/niet-dev-k1921vg1t/timer_blink/](../../../../examples/niet-dev-k1921vg1t/timer_blink/).
- [UART1 echo](example-uart1-echo.md) — echo bytes on UART1 via an
  external USB-UART dongle (XP2 pins 1/3), sources in
  [examples/niet-dev-k1921vg1t/uart1_echo/](../../../../examples/niet-dev-k1921vg1t/uart1_echo/).
- [PCA9685 PWM driver](example-i2c-pca9685.md) — drive an NXP PCA9685
  16-channel PWM controller over I2C0 (XP1 pins 5/3), sources in
  [examples/niet-dev-k1921vg1t/pca9685/](../../../../examples/niet-dev-k1921vg1t/pca9685/).
- [HC-SR04 ultrasonic](example-hcsr04.md) — distance via a TMR1-timed
  echo pulse; requires 5 V + a 10 kΩ/20 kΩ divider (not verified on this
  board without resistors), sources in
  [examples/niet-dev-k1921vg1t/hcsr04/](../../../../examples/niet-dev-k1921vg1t/hcsr04/).
- [PWM output](example-pwm.md) — PWM on PWM1 (TMR1) via the niiet PWM
  driver (XP1 pin 19), sources in
  [examples/niet-dev-k1921vg1t/pwm_demo/](../../../../examples/niet-dev-k1921vg1t/pwm_demo/).
- [DMA copy](example-dma.md) — memory-to-memory DMA copy via DMA0 (no
  wiring), sources in
  [examples/niet-dev-k1921vg1t/dma_copy/](../../../../examples/niet-dev-k1921vg1t/dma_copy/).
- [ADC temperature sensor](example-adc-tsensor.md) — read the internal
  temperature sensor via the ADC (channel 63, no wiring), sources in
  [examples/niet-dev-k1921vg1t/adc_tsensor/](../../../../examples/niet-dev-k1921vg1t/adc_tsensor/).
- [Sharp distance sensor](example-sharp-2y0a21.md) — Sharp GP2Y0A21 IR
  distance on ADC channel 8 (PA8, XP1 pin 27), sources in
  [examples/niet-dev-k1921vg1t/sharp_2y0a21/](../../../../examples/niet-dev-k1921vg1t/sharp_2y0a21/).
- [SDRAM bring-up](example-sdram.md) — bring up the 32 MB external SDRAM
  and verify it (no wiring), sources in
  [examples/niet-dev-k1921vg1t/sdram/](../../../../examples/niet-dev-k1921vg1t/sdram/).
- [RTC](example-rtc.md) — real-time clock via the Embox RTC framework
  (CR1220 battery backup), driver in
  [examples/niet-dev-k1921vg1t/rtc/](../../../../examples/niet-dev-k1921vg1t/rtc/).

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
- Board documentation (rev 1.1): <https://gitflic.ru/project/niiet_hardware/441461_050>
- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (chapters 9, 13, 41)
