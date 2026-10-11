# GPIO

Seven 16-bit ports GPIOA-GPIOG (112 pins total). Every pin is a
bidirectional general-purpose I/O with alternate functions; mode, alternate
function, pull-up, and open-drain/open-source can be configured
independently per pin. After reset, all port pins are high-impedance
general-purpose inputs.

Port base addresses: GPIOA `4000_0000h` ... GPIOG `4000_6000h`
(step 1000h).

## Register map (per port)

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | DATA | input data |
| +04h | DATAOUT | output data |
| +08h | DATAOUTSET | set output bits (write 1 = set) |
| +0Ch | DATAOUTCLR | clear output bits |
| +10h | DATAOUTTGL | toggle output bits |
| +20h | PULLMODE | pull mode selection |
| +24h | OUTMODE | output mode selection |
| +2Ch | OUTENSET | enable output control |
| +30h | OUTENCLR | disable output control |
| +34h | ALTFUNCSET | enable alternate function |
| +400h | MASKLB | low-byte mask registers (array) |
| ... | MASKLB/MASKHB | masking and locking mechanisms |

Ports support interrupt generation (interrupt vectors 8-14) and hardware
request generation for DMA/peripherals, plus configuration lock and mask
mechanisms.

## Embox GPIO driver

The Embox `k1921vg1t_dev` template includes
`embox.driver.gpio.niiet_gpio(altfunc_field_width=4)` and exposes the
ports through the standard GPIO API (`src/drivers/gpio/gpio.h`). Board
usage: [LED/button counter example](КФДЛ.441461.050/example-led-button-counter.md).

## GPIO interrupt quirk (observed on the dev board)

On the dev board, writing the GPIO edge-configuration registers
(`INTTYPESET`/`INTEDGECLR`/`INTPOLCLR`) latches a spurious `INTSTATUS`
bit; with the interrupt enabled, that keeps the line asserted and the
PLIC re-pends after every claim — an interrupt storm (the RISC-V trap
handler, which nests interrupts during dispatch, then smashes the
stack). A both-edges configuration on an idle-high pin latches at
enable and breaks boot. `INTSTATUS` write-1-to-clear itself works (per
the UM); the trigger is the configuration write.

The workaround: configure the interrupt once, then mask/clear/re-arm by
writing only `INTENCLR`/`INTSTATUS`/`INTENSET` — never re-run
`gpio_setup_mode()`. Additionally, the `niiet_gpio` driver never records
the attached IRQ handler, so repeated `gpio_setup_mode()` calls silently
disable the interrupt. Details, GDB evidence, and a driver patch:
[experiment journal](КФДЛ.441461.050/experiment-led-button-counter.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 15 and appendix A.9
