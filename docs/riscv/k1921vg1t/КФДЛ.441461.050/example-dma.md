# Example: DMA memory-to-memory copy

Copy a buffer from RAM to RAM using DMA0, without CPU involvement, then
verify the result. No wiring needed.

> **Status: verified on hardware (2026-10-13)** — `dma_copy` copied
> 512 bytes via DMA0 channel 0 and verified the result; re-runs work.
> See the [experiment journal](experiment-dma.md).

## Build and run

1. Write the command module in `<embox>/src/cmds/example/dma_copy/` (see
   [Commands and drivers](../../../embox/commands-drivers.md)).
2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, add the DMA
   driver (it is **not** in the stock template) and the command:

   ```
   include embox.driver.dma.niiet_dma_drv
   include embox.cmd.example.dma_copy.dma_copy
   ```
3. Build, flash, reset.
4. On the console:

   ```
   root@embox:/#dma_copy
   dma_copy: 512 bytes via DMA0 ch0: OK
   ```

## What the command does

1. `dma_dev_by_id(0)` → the DMA0 device, `dma_init()` enables the clock.
2. `dma_config()` attaches the channel 0 interrupt (after `irq_detach()`
   to handle re-runs).
3. Fills a `struct dma_req`: source/destination addresses, 32-bit width
   (4), increment, `DMA_TYPE_MEM` requestor on both sides, 512 bytes.
4. `dma_transfer()` programs the channel, `dma_activate()` starts it.
5. Waits for the completion interrupt (callback), then compares the
   destination against the source.

## Verification checklist (on hardware)

1. `dma_copy` prints `OK` (destination matches source). ✔ (2026-10-13)
2. Run it again — still `OK` (the IRQ re-attach is handled). ✔

## Notes

- The NDTL `BUFFER_SIZE` field is 10 bits, so one linear-mode transfer
  moves at most ~1 KB.
- The niiet DMA IRQ handler returns 0 rather than `IRQ_HANDLED` (1); it
  works on this board, but is worth flagging upstream.

## Sources

- DMA chapter: [dma.md](../dma.md).
- Embox DMA API: `src/drivers/dma/dma_new.h`, `src/drivers/dma/niiet_dma/`.
