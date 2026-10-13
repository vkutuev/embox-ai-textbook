# UART1 echo example for NIIET-DEV-K1921VG1T

Echo bytes received on UART1 back to the same device, using a USB-UART
dongle wired to the board's XP2 connector. The console (UART0/ttyS0)
stays independent.

## Hardware

| Board (XP2) | MCU pin | Alt fn | Dongle pin |
| ----------- | ------- | ------ | ---------- |
| pin 1 | PB4 | 3 (UART1_RX) | **TXD** |
| pin 3 | PB5 | 3 (UART1_TX) | **RXD** |
| GND (any board GND, e.g. XP7 pin 3) | — | — | **GND** |

- Dongle logic level **3.3 V** (the MCU pins are not 5V-tolerant).
- Baud **115200 8N1**.
- The board config already defines UART1 (pins, IRQ 67, clock `CLK_UART1`);
  this example only enables the `pl011_ttyS1` module.

## Install into Embox

1. Copy this directory into an Embox checkout:

   ```console
   $ cp -r uart1_echo/ <embox>/src/cmds/example/
   ```

2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the lines
   from `mods.conf.snippet` (the `pl011_ttyS1` module and the command).

3. Build, flash, reset.

## Run and verify

1. Open a serial terminal on the dongle at 115200 8N1.
2. On the board's console, run `uart1_echo`.
3. Type `hello` in the dongle terminal — it comes back echoed.
4. The console (`embox>` prompt) stays independent.

If the echo is garbled, check UART1's clock (RCU `UARTCLKCFG1` — should
be HSE 27 MHz like UART0; the pl011 driver uses one shared
`uartclk=27000000`).
