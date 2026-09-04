# Memory Map

The microcontroller starts executing code at address `0000_0000h`. The 32-bit
address space is divided into the following regions:

| Address range | Description |
| ------------- | ----------- |
| `FE00_0000h - FFFF_FFFFh` | PLIC system region |
| `8000_0000h - 81FF_FFFFh` | External SDRAM memory |
| `6000_0000h - 6FFF_FFFFh` | External memory (8 windows x 16 MiB) |
| `4000_0000h - 52FF_FFFFh` | Peripheral control registers |
| `2000_0000h - 200F_FFFFh` | Internal SRAM 1 MiB with ECC |
| `0040_0000h - 0047_FFFFh` | FlashD 512 KiB with ECC |
| `0000_0000h - 003F_FFFFh` | FlashM 4 MiB with ECC |

## Peripheral register map

| Address range | Block | Description |
| ------------- | ----- | ----------- |
| `5201_C000h - 5201_CFFFh` | MSC | MSC controller |
| `5201_B000h - 5201_BFFFh` | I2C3 | I2C interface 3 |
| `5201_A000h - 5201_AFFFh` | I2C2 | I2C interface 2 |
| `5201_9000h - 5201_9FFFh` | I2C1 | I2C interface 1 |
| `5201_8000h - 5201_8FFFh` | I2C0 | I2C interface 0 |
| `5201_7000h - 5201_7FFFh` | UART7 | UART 7 |
| `5201_6000h - 5201_6FFFh` | UART6 | UART 6 |
| `5201_5000h - 5201_5FFFh` | UART5 | UART 5 |
| `5201_4000h - 5201_4FFFh` | UART4 | UART 4 |
| `5201_3000h - 5201_3FFFh` | UART3 | UART 3 |
| `5201_2000h - 5201_2FFFh` | UART2 | UART 2 |
| `5201_1000h - 5201_1FFFh` | UART1 | UART 1 |
| `5201_0000h - 5201_0FFFh` | UART0 | UART 0 |
| `5200_F000h - 5200_FFFFh` | SPI7 | SPI interface 7 |
| `5200_E000h - 5200_EFFFh` | SPI6 | SPI interface 6 |
| `5200_D000h - 5200_DFFFh` | SPI5 | SPI interface 5 |
| `5200_C000h - 5200_CFFFh` | SPI4 | SPI interface 4 |
| `5200_B000h - 5200_BFFFh` | SPI3 | SPI interface 3 |
| `5200_A000h - 5200_AFFFh` | SPI2 | SPI interface 2 |
| `5200_9000h - 5200_9FFFh` | SPI1 | SPI interface 1 |
| `5200_8000h - 5200_8FFFh` | SPI0 | SPI interface 0 |
| `5200_7000h - 5200_7FFFh` | LIN7 | LIN interface 7 |
| `5200_6000h - 5200_6FFFh` | LIN6 | LIN interface 6 |
| `5200_5000h - 5200_5FFFh` | LIN5 | LIN interface 5 |
| `5200_4000h - 5200_4FFFh` | LIN4 | LIN interface 4 |
| `5200_3000h - 5200_3FFFh` | LIN3 | LIN interface 3 |
| `5200_2000h - 5200_2FFFh` | LIN2 | LIN interface 2 |
| `5200_1000h - 5200_1FFFh` | LIN1 | LIN interface 1 |
| `5200_0000h - 5200_0FFFh` | LIN0 | LIN interface 0 |
| `5101_7000h - 5101_7FFFh` | TMR15 | Timer 15 |
| `5101_6000h - 5101_6FFFh` | TMR14 | Timer 14 |
| `5101_5000h - 5101_5FFFh` | TMR13 | Timer 13 |
| `5101_4000h - 5101_4FFFh` | TMR12 | Timer 12 |
| `5101_3000h - 5101_3FFFh` | TMR11 | Timer 11 |
| `5101_2000h - 5101_2FFFh` | TMR10 | Timer 10 |
| `5101_1000h - 5101_1FFFh` | TMR9 | Timer 9 |
| `5101_0000h - 5101_0FFFh` | TMR8 | Timer 8 |
| `5100_F000h - 5100_FFFFh` | TMR7 | Timer 7 |
| `5100_E000h - 5100_EFFFh` | TMR6 | Timer 6 |
| `5100_D000h - 5100_DFFFh` | TMR5 | Timer 5 |
| `5100_C000h - 5100_CFFFh` | TMR4 | Timer 4 |
| `5100_B000h - 5100_BFFFh` | TMR3 | Timer 3 |
| `5100_A000h - 5100_AFFFh` | TMR2 | Timer 2 |
| `5100_9000h - 5100_9FFFh` | TMR1 | Timer 1 |
| `5100_8000h - 5100_8FFFh` | TMR0 | Timer 0 |
| `5100_7000h - 5100_7FFFh` | I2S1 | I2S interface 1 |
| `5100_6000h - 5100_6FFFh` | I2S0 | I2S interface 0 |
| `5100_5000h - 5100_5FFFh` | DAC1 | DAC 1 |
| `5100_4000h - 5100_4FFFh` | DAC0 | DAC 0 |
| `5100_3000h - 5100_3FFFh` | ACMP3 | Analog comparators 3 |
| `5100_2000h - 5100_2FFFh` | ACMP2 | Analog comparators 2 |
| `5100_1000h - 5100_1FFFh` | ACMP1 | Analog comparators 1 |
| `5100_0000h - 5100_0FFFh` | ACMP0 | Analog comparators 0 |
| `5001_C000h - 5001_CFFFh` | PWM15 | PWM unit 15 |
| `5001_B000h - 5001_BFFFh` | PWM14 | PWM unit 14 |
| ... | ... | ... |
| `5000_D000h - 5000_DFFFh` | PWM0 | PWM unit 0 |
| `5000_C000h - 5000_CFFFh` | FLASHD | FlashD controller with OTP |
| `5000_B000h - 5000_BFFFh` | ETH | Ethernet controller |
| `5000_A000h - 5000_AFFFh` | SDRAM | SDRAM interface |
| `5000_9000h - 5000_9FFFh` | EMC | External memory controller |
| `5000_8000h - 5000_8FFFh` | TRNG | Random number generator |
| `5000_7000h - 5000_7FFFh` | SRAM | SRAM ECC control registers |
| `5000_6000h - 5000_6FFFh` | WDT | Watchdog timer |
| `5000_5000h - 5000_5FFFh` | RTC | Real-time clock |
| `5000_4000h - 5000_4FFFh` | RCU | Reset and clock control |
| `5000_3000h - 5000_3FFFh` | SIU | System management unit |
| `5000_2000h - 5000_2FFFh` | FLASHM | FlashM controller |
| `5000_0000h - 5000_1FFFh` | DMA | 32-channel DMA controller |
| `4109_1000h - 4109_1FFFh` | USB1H | USB 1 host controller |
| `4109_0000h - 4109_0FFFh` | USB1D | USB 1 device controller |
| `4108_1000h - 4108_1FFFh` | USB0H | USB 0 host controller |
| `4108_0000h - 4108_0FFFh` | USB0D | USB 0 device controller |
| `4107_0000h - 4107_0FFFh` | CRYPTO | Crypto block |
| `4106_0000h - 4106_0FFFh` | HASH | Hash processor |
| `4105_0000h - 4105_0FFFh` | CRC | CRC block |
| `4104_0000h - 4104_0FFFh` | QSPI1 | QSPI interface 1 |
| `4103_0000h - 4103_0FFFh` | QSPI0 | QSPI interface 0 |
| `4102_0000h - 4102_FFFFh` | ADC | ADC block |
| `4101_0000h - 4101_FFFFh` | CAN | CAN 2.0B controller |
| `4100_3000h - 4100_3FFFh` | CANFD3 | CAN FD 3 |
| `4100_2000h - 4100_2FFFh` | CANFD2 | CAN FD 2 |
| `4100_1000h - 4100_1FFFh` | CANFD1 | CAN FD 1 |
| `4100_0000h - 4100_0FFFh` | CANFD0 | CAN FD 0 |
| `4000_6000h - 4000_6FFFh` | GPIOG | Port G |
| `4000_5000h - 4000_5FFFh` | GPIOF | Port F |
| `4000_4000h - 4000_4FFFh` | GPIOE | Port E |
| `4000_3000h - 4000_3FFFh` | GPIOD | Port D |
| `4000_2000h - 4000_2FFFh` | GPIOC | Port C |
| `4000_1000h - 4000_1FFFh` | GPIOB | Port B |
| `4000_0000h - 4000_0FFFh` | GPIOA | Port A |

## ECC

Internal SRAM supports Hamming-code ECC (corrects one error, detects two).
Each byte is protected by four correction bits; a 32-bit word by 20 bits.
Enable ECC with the `EN` bit of the SRAM `CFG` register; single/double error
counters are `SEU_CNTR`/`DEU_CNTR` and addresses `SEU_ADDR`/`DEU_ADDR`.
Flash controllers support the same scheme (64 data bits protected by 8 ECC
bits).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 8
