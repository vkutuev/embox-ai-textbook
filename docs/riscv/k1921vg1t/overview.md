# K1921VG1T Overview

K1921VG1T is a 32-bit microcontroller by АО НИИЭТ (NIET, Voronezh) based on
the dual-core SCR5 RISC-V CPU. K1921VG1T (LQFP-208) and K1921VG10Я (BGA-216)
share the same die and identical peripherals; they differ only in the package.

## Key features

- **Core**: dual-core SCR5, RV32IMAFDCP (Zicsr), including P extension for
  DSP/SIMD instructions, single- and double-precision FPU (IEEE 754-2008),
  up to 204 MHz.
- **Memory**: 4 MiB main flash (FlashM), 512 KiB data flash (FlashD),
  16 KiB OTP, 1 MiB SRAM with ECC; L1 I/D caches 32 KiB each, shared 128 KiB
  L2 cache; external memory controllers EMC (up to 128 MiB) and SDRAM
  (up to 32 MiB).
- **Peripherals**:
  - 8 UART, 8 SPI, 2 QSPI, 2 I2S, 8 LIN, 4 I2C, CAN 2.0B (4 nodes),
    4 CAN FD, USB 2.0 FS/HS (device + host, integrated FS PHY),
    Ethernet 10/100/1000 with PHY, MSC (Micro Second Channel).
  - 16 32-bit timers (TMR0-TMR15) with capture/compare.
  - 16 two-channel PWM units (PWM0-PWM15).
  - 48-channel 12-bit ADC, 2 DAC, 4 analog comparators (ACMP), temperature
    sensor (TSENSOR).
  - TRNG, HASH (SHA-1/224/256, MD5), CRC (CRC7/8/16/32), CRYPTO
    (AES-128/256, Kuznechik, Magma).
  - WDT, RTC with battery domain.
- **I/O**: 7 x 16-bit GPIO ports (A-G) — 112 pins with alternate functions.
- **Power**: 3.0-3.6 V digital/analog, 1.6-3.6 V battery domain (VBAT),
  POR/BOR supervisors, operating range -40 to +105 °C.
- **Debug**: JTAG (TCK, TMS, TDI, TDO, TRST); supported adapters:
  Olimex ARM-USB-OCD-H, J-Link V9, CMSIS-DAP, FT2232-based.

## Reset and pins of note

- `RST#` must be held high; external reset is a low pulse of at least 10 µs.
- `SERVEN` high during reset puts flash into service-erase mode (full erase
  via JTAG only).
- After reset, GPIO pins are configured as general-purpose inputs
  (high-impedance).

## Compiler flags

```
-march=rv32imafdcp_zicsr -mabi=ilp32d
```

The core supports machine, supervisor, and user privilege modes, and MMU with
32-entry ITLB/DTLB plus a 16-entry MPU, so it can run Linux-like systems
(Embox included) in supervisor mode.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, sections 1-3
- `doc/Start_NIIET_RISCV.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, section 2.1
