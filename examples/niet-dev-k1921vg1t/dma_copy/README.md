# DMA memory-to-memory copy example for NIIET-DEV-K1921VG1T

Copy a buffer from RAM to RAM using the DMA controller (DMA0), without
CPU involvement, then verify the result. No wiring needed.

## Install into Embox

1. Copy `dma_copy/` into `<embox>/src/cmds/example/`.
2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the lines
   from `mods.conf.snippet` — the DMA driver (`niiet_dma_drv`) is *not*
   in the stock template, so it must be added.
3. Build, flash, reset.

## Run and verify

```
root@embox:/#dma_copy
dma_copy: 512 bytes via DMA0 ch0: OK
```

The command fills a source buffer with a pattern, configures a DMA
channel (source/destination addresses, 32-bit width, increment, memory
requestor), triggers the transfer, waits for the channel interrupt, and
verifies the destination matches the source.

The NDTL register's `BUFFER_SIZE` field is 10 bits, so one linear-mode
transfer moves at most ~1 KB; larger buffers need multiple transfers.
