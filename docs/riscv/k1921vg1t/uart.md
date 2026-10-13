# UART

Eight identical UARTs (UART0-UART7), each with a 12-bit receive FIFO and an
8-bit transmit FIFO, up to 32 bytes each (configurable as single-byte).
Data width 5-8 bits, speeds up to 3000 kbit/s, parity, 1-2 stop bits, break
detection, independent maskable interrupts, DMA support.

## Register map

Base addresses:

```
UART0  5201_0000h   UART1  5201_1000h   UART2  5201_2000h   UART3  5201_3000h
UART4  5201_4000h   UART5  5201_5000h   UART6  5201_6000h   UART7  5201_7000h
```

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | DR | data: write = TX byte, read = RX byte (bits 11-8 hold RX error flags OE/BE/PE/FE) |
| +04h | RSR | receiver status/error clear (OE, BE, PE, FE; write 1 to clear) |
| +18h | FR | flags: RI, TXFE, RXFF, TXFF, RXFE, BUSY, DCD, DSR, CTS |
| +24h | IBRD | integer baud-rate divider |
| +28h | FBRD | fractional baud-rate divider |
| +2Ch | LCRH | line control: SPS, WLEN, FEN, STP2, EPS, PEN, BRK |
| +30h | CR | control: UARTEN, TXE, RXE, ... |

The remaining registers (IMSC/RIS/MIS/ICR, DMACR, etc.) are described in
appendix A.22 of the user manual.

## Clocking and reset

- Release the module from reset by setting the `RSTDIS` bit in the
  corresponding `UARTCFGi` register of the RCU block.
- Constraint: `fUARTCLK <= fPCLK * 5/3`. For 921600 baud
  (`fUARTCLK = 14.7456 MHz`), `fPCLK` must be at least 8.84736 MHz.

## Baud-rate configuration

The divider is split into integer (IBRD) and fractional (FBRD) parts:

```
K = fUARTCLK / (16 * baudrate)
IBRD = integer part of K
FBRD = round(fractional part of K * 64)
```

Example: `fUARTCLK = 4 MHz`, baudrate 230400 -> K = 1.085 -> IBRD = 1,
FBRD = 5 (real baudrate 231911, error 0.656%). Maximum setting error is
1.5625%.

`IBRD`, `FBRD`, and `LCRH` may only be changed while the UART is disabled
and no byte is being transmitted/received. New divider values take effect
only after a subsequent write to `LCRH`.

## Embox driver

The Embox `k1921vg1t_dev` template uses the `pl011` driver. The board
config (`platform/niiet/board_config/niiet_k1921vg1t_dev.conf.h`) already
defines UART0 and UART1 (pins, IRQ, clock, baud); adding
`embox.driver.serial.pl011_ttyS<N>` registers `/dev/ttyS<N>`, configures
the pins (PB0/PB1 for UART0, PB4/PB5 for UART1, alt function 3), and
enables the clock. The driver uses one shared `uartclk` option
(27 MHz HSE on the dev board). Board usage:
[UART1 echo example](КФДЛ.441461.050/example-uart1-echo.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 29 and appendix A.22
