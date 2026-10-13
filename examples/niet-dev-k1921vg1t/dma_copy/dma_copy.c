/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: memory-to-memory DMA copy.
 *
 * Uses DMA0 (32 channels, 0x50000000) to copy a block of RAM from a
 * source buffer to a destination buffer without CPU involvement, then
 * verifies the result. Demonstrates the DMA controller setup and the
 * Embox "new" DMA API (`dma_req` + `dma_transfer` + `dma_activate`).
 *
 * The niiet DMA driver completes transfers with a channel interrupt;
 * this example registers a callback and waits for it.
 *
 * Note: the NDTL register's BUFFER_SIZE field is 10 bits, so a single
 * transfer moves at most ~1 KB (linear mode).
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <drivers/dma.h>
#include <kernel/irq.h>

#include <config/board_config.h>

#define DMA_DEV_ID  0
#define DMA_CH      0
#define DMA_CH_IRQ  CONF_DMA0_IRQ_CHAN0

#define N_WORDS     128          /* 512 bytes, well within NDTL */
#define N_BYTES     (N_WORDS * 4)

static uint32_t src[N_WORDS] __attribute__((aligned(4)));
static uint32_t dst[N_WORDS] __attribute__((aligned(4)));

static volatile int done;

static int dma_cb(struct dma_req *req, void *data, int res) {
	done = 1;
	return 0;
}

int main(int argc, char **argv) {
	struct dma_dev *dev;
	struct dma_config conf;
	struct dma_req req;
	int mem_type;
	int i, errors;

	dev = dma_dev_by_id(DMA_DEV_ID);
	if (dev == NULL) {
		fprintf(stderr, "dma_copy: no DMA%d (is niiet_dma_drv enabled?)\n",
		    DMA_DEV_ID);
		return -1;
	}

	dma_init(dev);

	for (i = 0; i < N_WORDS; i++) {
		src[i] = 0xA5A50000u | (unsigned)i;
	}
	memset(dst, 0, N_BYTES);

	mem_type = dma_get_type(dev, DMA_TYPE_MEM);

	/* attach the channel interrupt; drop a stale registration from a
	 * previous run first (dma_config() attaches the IRQ and leaks it) */
	irq_detach(DMA_CH_IRQ, dev);

	memset(&conf, 0, sizeof(conf));
	if (dma_config(dev, DMA_CH, &conf) < 0) {
		fprintf(stderr, "dma_copy: dma_config failed\n");
		return -1;
	}

	memset(&req, 0, sizeof(req));
	req.dr_chan = DMA_CH;
	req.dr_src = (uintptr_t)src;
	req.dr_src_width = 4;       /* 32-bit */
	req.dr_src_inc = 1;
	req.dr_src_type = mem_type; /* memory requestor */
	req.dr_dest = (uintptr_t)dst;
	req.dr_dest_width = 4;
	req.dr_dest_inc = 1;
	req.dr_dest_type = mem_type;
	req.dr_size = N_BYTES;
	req.dr_callback = dma_cb;

	done = 0;
	if (dma_transfer(dev, DMA_CH, &req) < 0) {
		fprintf(stderr, "dma_copy: dma_transfer failed\n");
		return -1;
	}
	dma_activate(dev, DMA_CH);

	/* wait for the completion interrupt */
	while (!done) {
		usleep(1000);
	}

	errors = 0;
	for (i = 0; i < N_WORDS; i++) {
		if (dst[i] != src[i]) {
			errors++;
		}
	}

	printf("dma_copy: %d bytes via DMA0 ch%d: %s\n", N_BYTES, DMA_CH,
	    errors ? "FAILED" : "OK");
	if (errors) {
		printf("  %d/%d words mismatch\n", errors, N_WORDS);
		return -1;
	}

	return 0;
}
