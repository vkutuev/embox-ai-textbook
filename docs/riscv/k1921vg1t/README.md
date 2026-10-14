# K1921VG1T

Documentation for the NIET (НИИЭТ) K1921VG1T microcontroller, distilled from
the official user manual, errata, and quick-start guide.

## Contents

- [Overview](overview.md) — MCU features, family, packages
- [Memory map](memory-map.md) — address space and peripheral register map
- [Core (SCR5)](core.md) — ISA, privilege modes, exceptions, MMU/MPU, caches,
  memory-mapped timer
- [Clocks (RCU)](clocks.md) — clock sources, PLL configuration
- [Timers (TMR)](timer.md) — timer register map, modes, interrupts
- [PWM](pwm.md) — PWM via the TMR units, CAPCOM compare, Embox driver
- [DMA](dma.md) — DMA controller, channel registers, memory-to-memory
- [ADC](adc.md) — ADC blocks, sequencers, internal temperature sensor
- [Interrupts (PLIC)](interrupts.md) — interrupt vectors, register map,
  programming model
- [Flash](flash.md) — FlashM/FlashD/OTP, config word, service erase
- [UART](uart.md) — UART register map and baud-rate setup
- [I2C](i2c.md) — I2C controller register map and Embox driver
- [GPIO](gpio.md) — GPIO port registers
- [Debugging](debugging.md) — JTAG, OpenOCD, toolchain, SDK
- [Errata](errata.md) — known silicon issues and workarounds

## Boards

- [КФДЛ.441461.050 (NIIET-DEV-K1921VG1T)](КФДЛ.441461.050/README.md) —
  dev board bring-up with Embox (verified Hello World run), flashing
  guide, and experiment records:
  [SDK OpenOCD configurations](КФДЛ.441461.050/experiment-sdk-openocd-configs.md),
  [GDB `load` flash](КФДЛ.441461.050/experiment-gdb-flash.md),
  [LED/button counter example](КФДЛ.441461.050/example-led-button-counter.md),
  [LED/button counter experiment](КФДЛ.441461.050/experiment-led-button-counter.md),
  [timer blink example](КФДЛ.441461.050/example-timer-blink.md),
  [timer blink experiment](КФДЛ.441461.050/experiment-timer-blink.md),
  [UART1 echo example](КФДЛ.441461.050/example-uart1-echo.md),
  [UART1 echo experiment](КФДЛ.441461.050/experiment-uart1-echo.md),
  [PCA9685 I2C example](КФДЛ.441461.050/example-i2c-pca9685.md),
  [PCA9685 I2C experiment](КФДЛ.441461.050/experiment-i2c-pca9685.md),
  [HC-SR04 example](КФДЛ.441461.050/example-hcsr04.md),
  [HC-SR04 experiment](КФДЛ.441461.050/experiment-hcsr04.md),
  [PWM example](КФДЛ.441461.050/example-pwm.md),
  [PWM experiment](КФДЛ.441461.050/experiment-pwm.md),
  [DMA example](КФДЛ.441461.050/example-dma.md),
  [DMA experiment](КФДЛ.441461.050/experiment-dma.md),
  [ADC temperature-sensor example](КФДЛ.441461.050/example-adc-tsensor.md),
  [ADC temperature-sensor experiment](КФДЛ.441461.050/experiment-adc-tsensor.md),
  [Sharp distance-sensor example](КФДЛ.441461.050/example-sharp-2y0a21.md),
  [Sharp distance-sensor experiment](КФДЛ.441461.050/experiment-sharp-2y0a21.md)

## Embox relevance

The most important facts for bringing up Embox on K1921VG1T:

| Concern | Answer |
| ------- | ------ |
| ISA flags | Embox template: `-march=rv32imfdc_zicsr_zifencei -mabi=ilp32d` (core is RV32IMAFDCP; the vendor recommends `rv32imafdcp_zicsr`) |
| Boot address | `0x0000_0000` (FlashM) |
| RAM | 1 MiB SRAM at `0x2000_0000` |
| Console UART | UART0 at `0x5201_0000` |
| Interrupt controller | PLIC at `0xFE00_0000`, 254 sources, 16 priority levels |
| Timer | memory-mapped MTIMER (per-core compare), 16 TMR units at `0x5100_8000`+ |
| Debug | JTAG; TCK must satisfy `fSYSCLK / fTCK >= 12` |

The microcontroller has two cores (DualCore or Lockstep mode) and supports
Linux (S-mode with MMU), so Embox can run in supervisor mode.

## Sources

All documents are published in the NIET documentation repository
<https://gitflic.ru/project/niiet/niiet>:

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` — К1921ВГ1Т, К1921ВГ10Я user
  manual (от 25.08.2026)
- `doc/K1921VG1T/Errata_K1921VG1T.pdf` — errata (от 27.02.2026)
- `doc/Start_NIIET_RISCV.pdf` — quick-start guide for NIET RISC-V MCUs
  (от 17.04.2026)
