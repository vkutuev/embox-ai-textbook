# Interrupt Controller (PLIC)

The platform-level interrupt controller lives in the system region at base
`FE00_0000h`. It has 254 peripheral interrupt sources and 16 priority
levels. Four targets (T) map to privilege modes of the two cores:

| Target | Meaning |
| ------ | ------- |
| T0 | core 0, M-mode |
| T1 | core 0, S-mode |
| T2 | core 1, M-mode |
| T3 | core 1, S-mode |

PLIC asserts the RISC-V machine external interrupt (MEI, `mip` bit 11) or
supervisor external interrupt (SEI, bit 9).

## Register map (excerpt)

| Address | Register | Description |
| ------- | -------- | ----------- |
| `FE00_0004 + 4*(N-1)` | SRC_PRI[N] | priority of source N (1-254) |
| `FE00_1000` | INT_PEND[0] | pending bits for sources 1-31 |
| `FE00_101C` | INT_PEND[7] | pending bits for sources 224-254 |
| `FE00_2000` | T0_INTEN[0] | enable for sources 1-31, target T0 |
| `FE00_2020` | T0_INTEN[8] | enable, target T0 continued |
| `FE00_2080` | T1_INTEN[0] | enable for sources 1-31, target T1 |
| `FE00_2100` | T2_INTEN[0] | enable for sources 1-31, target T2 |
| `FE00_2180` | T3_INTEN[0] | enable for sources 1-31, target T3 |
| `FE1F_0004 + 4*(N-1)` | SRC_MODE[N] | trigger mode of source N |
| `FE20_0000` | PRI_TRSHLD0 | priority threshold, target T0 |
| `FE20_0004` | START_END0 | start/end interrupt, target T0 |
| `FE20_1000` | PRI_TRSHLD1 | priority threshold, target T1 |
| `FE20_1004` | START_END1 | start/end interrupt, target T1 |
| `FE20_2000` | PRI_TRSHLD2 | priority threshold, target T2 |
| `FE20_2004` | START_END2 | start/end interrupt, target T2 |
| `FE20_3000` | PRI_TRSHLD3 | priority threshold, target T3 |
| `FE20_3004` | START_END3 | start/end interrupt, target T3 |

`SRC_MODE` values: 0x0 off, 0x1 high level, 0x2 low level, 0x3 rising
edge, 0x4 falling edge, 0x5 dual edge. Typically write 0x1 to enable a
source.

## Interrupt vectors (selection)

| Vector | Source | Description |
| ------ | ------ | ----------- |
| 1 | WDT | watchdog timer |
| 2 | RCU | clock/reset unit |
| 3 | RTC | real-time clock |
| 4 | FLASH | flash controller |
| 5 | SRAM | SRAM controller |
| 6 | L2_ERROR | L2 cache error |
| 7 | SIU | system management unit |
| 8-14 | GPIOA-GPIOG | GPIO ports |
| 15 | ETH | Ethernet |
| 16-31 | TMR0-TMR15 | timer general interrupts |
| 32-63 | DMA_CH0-DMA_CH31 | DMA channel interrupts |
| 64,67,...,85 | UART0-UART7 | UART general interrupts |
| 65,68,...,86 | UARTx_TD | UART transmit done |
| 66,69,...,87 | UARTx_RX | UART receive FIFO |
| 88-119 | TMR0_CC0-TMR3_CC7 | capture/compare units |
| 120-143 | SPI0-SPI7 (general/RX/TX) | SPI interrupts |
| 144-151 | LIN0-LIN7 | LIN interrupts |
| 152-155 | TMR0-TMR3_PERIOD | timer period interrupts |
| 156-159 | I2C0-I2C3 | I2C interrupts |
| 160-163 | I2S0-I2S1 (TX/RX) | I2S interrupts |
| 164-165 | USB0-USB1 | USB interrupts |
| 166-167 | QSPI0-QSPI1 | QSPI interrupts |
| 170-181 | CANFD0-CANFD3 (general/timer/ECC) | CAN FD interrupts |
| 182-197 | CAN_0-CAN_15 | CAN 2.0B lines |
| 198-205 | ADC_SEQ0-ADC_SEQ7 | ADC sequencers |
| 206 | ADC_DC | ADC digital comparators |
| 207-210 | ACMP0-ACMP3 | analog comparators |
| 211 | TRNG | random number generator |
| 212 | HASH | hash processor |
| 213 | CRYPTO | crypto block |
| 214-218 | RTC_ALARM1/2, RTC_BATTERY, RTC_N_POWER, RTC_OVERFLOW | RTC events |
| 219-251 | PWMx_TZ, PWM0-PWM15 (+HD) | PWM interrupts |
| 252-254 | MSC, MSC_upstream, MSC_downstream | MSC interrupts |

## Programming model

1. Write the source priority to `SRC_PRI[n]` (0 after reset = disabled;
   valid priorities 1-7).
2. Write 0x1 (high level) to `SRC_MODE[n]`.
3. Set the corresponding bit in `Tn_INTEN[]` for the desired target.
4. Set the threshold `PRI_TRSHLDn` (default 0).
5. In the RISC-V trap handler, enable MEIE in `mie` and MIE in `mstatus`.
6. Claim the interrupt by reading `START_ENDn` (returns the ID of the
   highest-priority pending interrupt, or 0).
7. On completion, write the ID back to `START_ENDn`.

If the same source is enabled on several targets of the same core, the
interrupt is handled in M-mode.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 13
