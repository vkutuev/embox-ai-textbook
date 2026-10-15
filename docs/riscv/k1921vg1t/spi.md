# SPI

Eight identical synchronous serial-port controllers (SPI0-SPI7) based on the
ARM PrimeCell PL022 SSP. Each supports Motorola SPI, Texas Instruments SSI,
and National Semiconductor Microwire framing, master or slave operation,
4-32 bit data widths, and independent transmit/receive FIFOs with
programmable interrupt thresholds and DMA requests.

## Register map

Base addresses:

```
SPI0  5200_8000h   SPI1  5200_9000h   SPI2  5200_A000h   SPI3  5200_B000h
SPI4  5200_C000h   SPI5  5200_D000h   SPI6  5200_E000h   SPI7  5200_F000h
```

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CR | control: DSS (data size = value+1), SCR (serial clock rate), FRF (framing), SPO/SPH, LBM (loopback), SSE (enable), MS (master/slave), SOD |
| +04h | — | reserved |
| +08h | DR | data: write = TX, read = RX |
| +0Ch | SR | status: TFE, TNF, RNE, RFF, BSY |
| +10h | CPSR | clock prescaler CPSDVSR (even value 2-254; bit 0 always 0) |
| +14h | IMSC | interrupt mask: RORIM, RTIM, RXIM, TXIM |
| +18h | RIS | raw interrupt status |
| +1Ch | MIS | masked interrupt status |
| +20h | ICR | interrupt clear (RORIC, RTIC) |
| +24h | DMACR | DMA control (RXDMAE, TXDMAE) |

The SCK frequency is:

```
fSCK = fSPICLK / (CPSDVSR * (1 + SCR))
```

## Framing and polarity

`CR.FRF` selects the frame format: `00` Motorola SPI, `01` TI SSI,
`10` Microwire. In Motorola mode `SPO` (clock polarity) and `SPH` (clock
phase) select the four SPI modes (0-3). `CR.LBM` enables internal loopback
(MOSI shorted to MISO) for testing without external wiring.

## Clocking and reset

Release the module from reset via the corresponding `SPICFGi` register of the
RCU block before use (same pattern as UART and I2C).

## Embox driver

Embox uses the `pl022` driver (`embox.driver.spi.pl022_spi`); the niiet
driver set also includes SPI. Candidate exercise with minimal wiring (one
jumper MOSI→MISO): SPI loopback — see the board
[development notes](КФДЛ.441461.050/development-notes.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (SPI chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`SPI_TypeDef`) and
  `tools/svd/K1921VG1T.svd` in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
