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

## GPIO interrupt notes (observed on the dev board)

The `niiet_gpio` driver never records the attached IRQ handler
(`gpio_irq_handler[port]` is checked but never written), so a second
`gpio_setup_mode()` call on the same port re-runs `irq_attach()`, fails
with `-EBUSY`, and returns before enabling the interrupt — silently
masking it. Fixed by one line (see the
[experiment journal](КФДЛ.441461.050/experiment-led-button-counter.md)).

An interrupt-driven button also produced an interrupt storm on press.
An early "re-config latch" hypothesis (that re-writing the edge-config
registers latches `INTSTATUS`) was tested with a register-level probe
and **not reproduced**: individual writes to `INTTYPESET`/`INTEDGECLR`/
`INTPOLCLR`/`INTENSET`/`INTENCLR` do not latch `INTSTATUS`, and
`INTSTATUS` write-1-to-clear works per the UM. The leading explanation
for the storm is button bounce amplified by the RISC-V trap handler's
nested dispatch (`ipl_enable()`). The workaround — mask the pin on the
first edge (`INTENCLR`), then clear `INTSTATUS` and re-arm `INTENSET` —
is verified to work. A both-edges configuration on an idle-high pin
(the `gpio_button` boot storm) is a separate, still-unexplained case.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 15 and appendix A.9
