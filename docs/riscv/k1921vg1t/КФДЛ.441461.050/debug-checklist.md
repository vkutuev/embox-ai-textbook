# Debug Checklist

Ordered diagnostic steps for the most common bring-up problems on this
board. Follow the order — each step is cheaper than the next.

## 0. Verify the wiring (before anything else)

Ask the user how the adapter is connected and compare with the verified
J-Link wiring in [README.md](README.md): nRST → J-Link RESET (pin 15),
+3V3 → VTref (pin 1), XP8 TRST → nTRST (pin 3). Symptoms of wrong wiring:
`adapter assert srst` does nothing (nRST on nTRST), DMI timeouts after
`init` (TRST wired while using `trst_and_srst`).

## 1. Console garbage: check the host side first

1. Confirm the device node exists and set the baud explicitly — the
   re-enumerated CH340B comes up at a default baud:

   ```console
   $ tools/uart_capture.sh /dev/ttyUSB0 115200 10
   ```

2. If garbage persists, capture a fresh boot log (step 3) at several
   bauds (115200, 57600, 230400, 921600) and look for readable text.

## 2. Console garbage: ask the MCU what it thinks

Dump the board state (halt, key registers) with:

```console
$ openocd -s <k1921vg1t_sdk>/tools/openocd/scripts -f tools/dump_state.cfg
```

Interpretation:

| Register | Address | Healthy value (Embox, 115200 console) |
| -------- | ------- | ------------------------------------- |
| Flash 0x0 | `0x00000000` | `20001197 ...` (Embox `_start`) |
| RCU UARTCFG0 | `0x50004050` | `0x111` — UARTCLK = HSE 27 MHz, enabled |
| RCU CLKSTAT | `0x5000402c` | SRC = 2 (PLL0), CLKGOOD0-2 = 1 |
| SIU CPESTAT | `0x50003008` | `0` (CPE = 0) |
| UART IBRD/FBRD | `0x52010024/28` | 14 / 41 → 115200 @ 27 MHz |
| UART LCRH | `0x5201002c` | `0x70` (8N1, FIFO) |
| UART CR | `0x52010030` | `0x301` (enabled, TX+RX) |
| GPIOB ALTFUNCNUM | `0x4000103c` | `0x33` (PB0/PB1 = alt 3) |
| GPIOB ALTFUNCSET | `0x40001034` | `0x03` |

If all MCU-side values are healthy, the problem is between the MCU TX
pin and the host (CH340B, cable, or host baud). Do not escalate to clock
theories before this point.

## 3. Deterministic output: reset and capture the boot log

```console
$ openocd -s <sdk>/tools/openocd/scripts -f interface/jlink.cfg \
    -c "transport select jtag" -c "adapter speed 100" \
    -c "reset_config srst_only srst_open_drain connect_deassert_srst" \
    -f target/k1921vg1t_singlecore.cfg \
    -c "init" -c "adapter assert srst" -c "sleep 300" \
    -c "adapter deassert srst" -c "shutdown"
```

while capturing the serial port (step 1). A clean boot log proves the
whole chain; garbage here with healthy registers points at the clock
configuration.

Note: `reset init`/`reset halt` always time out on this chip
("Hart didn't leave reset"; the debug module never reports
`havereset`) — use the manual SRST pulse above or `ndmreset`
(`riscv dmi_write 0x10 0x80000003` / `0x80000001`).

## 4. Only now: clock-domain theories

If and only if steps 1-3 all pass and the problem persists, investigate
clock domains (UARTCLK source, CPE, PLL settings, crystal) — see
[clocks.md](../clocks.md).
