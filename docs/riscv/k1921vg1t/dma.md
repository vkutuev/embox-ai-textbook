# DMA

The VG1T has one DMA controller (DMA0) with 32 channels that can move
data between memory and memory, or between memory and a peripheral
(UART, SPI, I2C, TMR, ADC, GPIO, ...). Each channel has independent
source/destination addresses, transfer widths, and increment control.

## Register map

DMA base `5000_0000h`. Each channel block is 0x80 bytes apart:
`CH[n] = 5000_0000h + n · 80h`.

| Offset (in channel) | Register | Description |
| ------------------- | -------- | ----------- |
| +00h | SRC_PTR | source address |
| +04h | DST_PTR | destination address |
| +08h | NDTL | number of data to transfer (linear mode); `BUFFER_SIZE` field is 10 bits |
| +0Ch | CONFIG | command flags (`CMD_LAST`, `CMD_SET_INT`) |
| +10h | STATIC0 | read side: `RD_BURST_MAX` (width in bytes), `RD_INCR` |
| +14h | STATIC1 | write side: `WR_BURST_MAX`, `WR_INCR` |
| +20h | STATIC4 | requestor numbers: `RD_PER_NUM`, `WR_PER_NUM` |
| +40h | CH_ACTIVE | channel active |
| +44h | CH_START | channel start |
| +50h | INT_RAWSTAT / +54h INT_CLEAR / +58h INT_ENABLE | interrupt |
| +60h | ERR_STOP / +64h ERR_RD_ADDR / +68h ERR_WR_ADDR | error status |

The transfer width is the burst size in bytes: 1 = 8-bit, 2 = 16-bit,
4 = 32-bit. The requestor numbers are the `*_DMA_REQUESTOR_IDX` values;
memory-to-memory uses `MEMORY_DMA_REQUESTOR_IDX` (0) for both source and
destination. The core registers (`CH_ENABLE`, `CONTROL`, priorities) sit
after the 32 channel blocks.

## Embox driver

The `niiet_dma_drv` driver (not enabled by the stock template) implements
the "new" DMA API (`<drivers/dma.h>`):

```
dma_dev_by_id(0) -> dma_init -> dma_config (attaches the channel IRQ)
-> fill struct dma_req -> dma_transfer -> dma_activate -> IRQ -> callback
```

`struct dma_req` carries `dr_src`/`dr_dest`, `dr_src_width`/`dr_dest_width`
(1/2/4), `dr_src_inc`/`dr_dest_inc`, `dr_src_type`/`dr_dest_type`
(requestor index, from `dma_get_type(dev, DMA_TYPE_MEM)`), `dr_size`
(bytes), and `dr_callback`. Completion is interrupt-driven
(`dma_complite()` invokes the callback); a re-run needs `irq_detach()`
first because `dma_config()` attaches the IRQ and leaks it.

Board usage:
[DMA copy example](КФДЛ.441461.050/example-dma.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (DMA chapter and appendix A).
- `src/drivers/dma/niiet_dma/` and `src/drivers/dma/dma_new.h` in
  <https://github.com/embox/embox>.
