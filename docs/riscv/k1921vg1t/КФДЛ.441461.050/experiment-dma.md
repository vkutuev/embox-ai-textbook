# Experiment: DMA memory-to-memory copy (2026-10-13)

Hardware verification of the [DMA example](example-dma.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console). No external hardware —
the transfer is RAM to RAM.

## Result

**Verified.** `dma_copy` copied 512 bytes via DMA0 channel 0 and the
destination matched the source. Re-runs also pass.

## Findings

- The stock `k1921vg1t_dev` template does **not** enable the DMA driver;
  `embox.driver.dma.niiet_dma_drv` must be added (its base address
  0x50000000 and the 32 channel IRQs come from the board config
  `dmas[]` entry).
- The driver implements the "new" DMA API (`<drivers/dma.h>`, which is
  `dma_new.h`): `dma_dev_by_id` / `dma_init` / `dma_config` /
  `dma_transfer` (a `struct dma_req`) / `dma_activate` / IRQ callback.
- Transfer width is the burst size in bytes (1/2/4); memory-to-memory
  uses `DMA_TYPE_MEM` (requestor 0) on both sides; `dr_size` is in bytes.
- **IRQ re-attach leak**: `dma_config()` calls `irq_attach()` and never
  detaches, so a second run fails with "Unknown error code". Fixed by
  `irq_detach(CONF_DMA0_IRQ_CHAN0, dev)` before `dma_config()`, mirroring
  the timer/GPIO examples.
- The niiet DMA IRQ handler returns `0` instead of `IRQ_HANDLED` (1);
  transfers still complete correctly on this board.

## Environment notes

- The CH340 console corrupts long output; short command output is
  reliable.
- A GDB attach halts the target; an SRST pulse recovers it.
