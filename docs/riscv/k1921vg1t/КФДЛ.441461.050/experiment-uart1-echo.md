# Experiment: UART1 echo (2026-10-13)

Hardware verification of the
[UART1 echo example](example-uart1-echo.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console at `/dev/ttyUSB0`,
USB-UART dongle on UART1).

## Result

**Verified**: typing `hello` on the dongle terminal echoes back; the
console (UART0/ttyS0) stays independent.

## Wiring (verified)

| Board (XP2) | MCU | Alt fn | Dongle |
| ----------- | --- | ------ | ------ |
| pin 1 | PB4 | UART1_RX | TXD |
| pin 3 | PB5 | UART1_TX | RXD |
| GND (XP7 pin 3) | — | — | GND |

Dongle at 3.3 V logic, 115200 8N1.

## Findings

- The board config (`niiet_k1921vg1t_dev.conf.h`) already defines UART1
  (pins PB5/PB4 alt 3, IRQ 67, clock `CLK_UART1`, baud 115200); the
  example only needs `embox.driver.serial.pl011_ttyS1` enabled plus a
  command that opens `/dev/ttyS1`.
- UART1 is set up **lazily on first `open()`** — before that the pins
  are not configured and `UART_CR` reads 0. (A debugger that halts the
  target can make this look like "pins not configured"; the setup runs
  when the device is actually opened.)
- After `open`, GDB confirmed: `GPIOB.ALTFUNCSET = 0x33` (PB0/PB1 for
  UART0 and PB4/PB5 for UART1), `UART1.CR = 0x301` (UARTEN|TXE|RXE),
  `LCRH = 0x70` (8-bit, FIFO).
- UART1 clock = HSE 27 MHz (the pl011 driver's shared `uartclk` value)
  — confirmed by the clean 115200 echo.
- No IRQ cleanup needed on Ctrl+C: the tty driver owns the IRQ (unlike
  the timer example, which had to detach its own).

## Environment notes

- The CH340 console re-enumerated to `/dev/ttyUSB0` (was `ttyUSB1`).
- After a GDB session the target stays halted (no auto-resume on
  disconnect); an SRST pulse recovers it.
