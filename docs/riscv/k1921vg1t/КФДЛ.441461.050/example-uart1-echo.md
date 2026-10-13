# Example: UART1 echo

Echo bytes on **UART1** using an external USB-UART dongle, while the
console keeps working on **UART0**. The point is to use a second UART
beyond the console, with its own device node (`/dev/ttyS1`).

## Hardware

| Board (XP2) | MCU pin | Alt fn | Dongle pin |
| ----------- | ------- | ------ | ---------- |
| pin 1 | PB4 | 3 (UART1_RX) | **TXD** |
| pin 3 | PB5 | 3 (UART1_TX) | **RXD** |
| GND (any board GND, e.g. XP7 pin 3) | — | — | **GND** |

- Dongle logic level **3.3 V** (the MCU pins are not 5V-tolerant, see
  the user manual table 3.6: `UIH` max is `UCC1 + 0.6 V`).
- Baud **115200 8N1** both sides.
- Optional: a logic analyzer on TX/RX/GND to watch the traffic.

## Embox support

The board config (`platform/niiet/board_config/niiet_k1921vg1t_dev.conf.h`)
already defines UART1: base `5201_1000h`, IRQ `PLIC_IRQ_UART1` (67),
pins PB5/PB4 (alt 3), clock `CLK_UART1`, baud 115200. The `pl011_ttyS1`
module (`src/drivers/serial/pl011/`) registers `/dev/ttyS1`; enabling it
configures the pins (via the board config) and the clock (via the RCU
driver's `clk_enable("CLK_UART1")`).

So the example only adds:

```
include embox.driver.serial.pl011_ttyS1(base_addr=0x52011000, irq_num=67)
include embox.cmd.example.uart1_echo.uart1_echo
```

## Build and run

1. Copy the example directory into an Embox checkout:

   ```console
   $ cp -r examples/niet-dev-k1921vg1t/uart1_echo/ <embox>/src/cmds/example/
   ```

2. Add the two `include` lines to
   `platform/niiet/templates/k1921vg1t_dev/mods.conf` (see
   `mods.conf.snippet` in the example directory).

3. Build, flash, reset.

4. Open a terminal on the dongle (115200 8N1), then on the console run:

   ```
   root@embox:/#uart1_echo
   uart1_echo: echoing /dev/ttyS1 (Ctrl+C to stop)
   ```

5. Type `hello` in the dongle terminal — it echoes back. The console
   stays independent.

## Verification checklist (on hardware)

1. Type in the dongle terminal → each character/line echoes back.
2. The console (`ttyS0`) still runs `version`, etc., unaffected.
3. (Optional) Capture TX/RX with the logic analyzer and confirm the
   115200 framing and the echo turnaround.
4. If output is garbled, check UART1's clock source (RCU `UARTCLKCFG1`,
   should be HSE 27 MHz like UART0) before suspecting wiring.

## Sources

- UART register map: [uart.md](../uart.md).
- Board config: `platform/niiet/board_config/niiet_k1921vg1t_dev.conf.h`
  (UART1 entry) in <https://github.com/embox/embox>.
