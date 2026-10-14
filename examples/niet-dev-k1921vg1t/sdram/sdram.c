/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: bring up the external SDRAM (32 MB).
 *
 * The board has 32 MB of SDRAM on the external memory controller (EMC),
 * mapped at 0x80000000, but it is unused until the controller is
 * programmed. Embox has no SDRAM driver, so this command configures the
 * pins, clock, and SDRAM controller directly, then verifies the memory
 * with a write/read test.
 *
 * Sequence (from the NIIET SDK bsp): mux the SDRAM pins to alt function 2
 * (ports D/E/F/G), enable the SDRAM clock (RCU SDRAMCFG, SIU EMMUXCTRL),
 * set MODE/CONTROL/REFRESH, enable, wait for INIT_DONE, then test.
 */

#include <stdio.h>

/* RCU / SIU */
#define RCU_CGCFGAHB    0x50004000UL
#define RCU_CGCFGAPB0   0x50004004UL
#define RCU_RSTDISAHB   0x50004010UL
#define RCU_RSTDISAPB0  0x50004014UL
#define RCU_SDRAMCFG    0x5000409CUL
#define SIU_EMMUXCTRL   0x50003060UL

/* SDRAM controller */
#define SDRAM_CTL_BASE  0x5000A000UL
#define SDRAM_TIMING    (SDRAM_CTL_BASE + 0x00)
#define SDRAM_REFRESH   (SDRAM_CTL_BASE + 0x04)
#define SDRAM_MODE      (SDRAM_CTL_BASE + 0x08)
#define SDRAM_CONTROL   (SDRAM_CTL_BASE + 0x0C)
#define SDRAM_STATUS    (SDRAM_CTL_BASE + 0x10)

/* GPIO port bases + offsets */
#define GPIOD           0x40003000UL
#define GPIOE           0x40004000UL
#define GPIOF           0x40005000UL
#define GPIOG           0x40006000UL
#define ALTFUNCSET      0x34
#define ALTFUNCNUM0     0x3C
#define ALTFUNCNUM1     0x40

/* RCU bits */
#define AHB_GPIO_PORTS  ((1u << 3) | (1u << 4) | (1u << 5) | (1u << 6))
#define APB0_SDRAMEN    (1u << 5)
#define SDRAMCFG_CLKEN   (1u << 0)
#define SDRAMCFG_RSTDIS  (1u << 4)
#define SDRAMCFG_DIVEN   (1u << 12)

/* SDRAM controller values (from the SDK bsp) */
#define MODE_VAL          (2u << 4)        /* CAS latency 2 */
#define CONTROL_VAL       ((1u << 10) | (3u << 16)) /* column 9-bit, req depth 3 */
#define CONTROL_ENABLE    (1u << 0)
#define REFRESH_VAL       ((2u << 12) | 120u) /* RF_MAX 2, RF_TIME 120 */
#define STATUS_INIT_DONE  (1u << 0)

#define MEM_SDRAM         0x80000000UL
#define SDRAM_SIZE        (32UL * 1024 * 1024)

#define RD(reg)           (*(volatile uint32_t *)(reg))
#define WR(reg, val)      (*(volatile uint32_t *)(reg) = (val))

static void settle_delay(void) {
	volatile unsigned int i;

	for (i = 0; i < 10000; i++) {
		__asm__ __volatile__("nop");
	}
}

/* mux all SDRAM signals (ports D/E/F/G) to alt function 2 */
static void sdram_pins(void) {
	WR(RCU_CGCFGAHB, RD(RCU_CGCFGAHB) | AHB_GPIO_PORTS);
	WR(RCU_RSTDISAHB, RD(RCU_RSTDISAHB) | AHB_GPIO_PORTS);

	/* D: addr[0:7] = D8-15, CLK = D7 */
	WR(GPIOD + ALTFUNCNUM0, RD(GPIOD + ALTFUNCNUM0) | 0x20000000);
	WR(GPIOD + ALTFUNCNUM1, RD(GPIOD + ALTFUNCNUM1) | 0x22222222);
	WR(GPIOD + ALTFUNCSET, RD(GPIOD + ALTFUNCSET) | 0xFF80);

	/* E: addr[8:12], BA, WE, RAS, CAS, CS, DQ, CKE (all 16 pins, alt 2) */
	WR(GPIOE + ALTFUNCNUM0, RD(GPIOE + ALTFUNCNUM0) | 0x22222222);
	WR(GPIOE + ALTFUNCNUM1, RD(GPIOE + ALTFUNCNUM1) | 0x22222222);
	WR(GPIOE + ALTFUNCSET, RD(GPIOE + ALTFUNCSET) | 0xFFFF);

	/* F: data[16:31] = F0-15 */
	WR(GPIOF + ALTFUNCNUM0, RD(GPIOF + ALTFUNCNUM0) | 0x22222222);
	WR(GPIOF + ALTFUNCNUM1, RD(GPIOF + ALTFUNCNUM1) | 0x22222222);
	WR(GPIOF + ALTFUNCSET, RD(GPIOF + ALTFUNCSET) | 0xFFFF);

	/* G: data[0:15] = G0-15 */
	WR(GPIOG + ALTFUNCNUM0, RD(GPIOG + ALTFUNCNUM0) | 0x22222222);
	WR(GPIOG + ALTFUNCNUM1, RD(GPIOG + ALTFUNCNUM1) | 0x22222222);
	WR(GPIOG + ALTFUNCSET, RD(GPIOG + ALTFUNCSET) | 0xFFFF);
}

static int sdram_init(void) {
	volatile uint32_t timeout;

	sdram_pins();

	/* SDRAM clock: PLL0, enable; SIU routes SDRAM to the external pins */
	WR(RCU_CGCFGAPB0, RD(RCU_CGCFGAPB0) | APB0_SDRAMEN);
	WR(RCU_RSTDISAPB0, RD(RCU_RSTDISAPB0) | APB0_SDRAMEN);
	WR(SIU_EMMUXCTRL, RD(SIU_EMMUXCTRL) | 1u);          /* SDRAMEN = SDRAM */

	WR(RCU_SDRAMCFG, SDRAMCFG_CLKEN | SDRAMCFG_DIVEN); /* CLKSEL=0, DIVN=0 */
	settle_delay();
	WR(RCU_SDRAMCFG, RD(RCU_SDRAMCFG) | SDRAMCFG_RSTDIS);

	/* SDRAM controller */
	WR(SDRAM_MODE, MODE_VAL);
	WR(SDRAM_CONTROL, CONTROL_VAL);
	WR(SDRAM_REFRESH, REFRESH_VAL);
	WR(SDRAM_CONTROL, CONTROL_VAL | CONTROL_ENABLE);

	timeout = 0;
	while (!(RD(SDRAM_STATUS) & STATUS_INIT_DONE)) {
		if (++timeout > 1000000) {
			return -1;
		}
	}

	return 0;
}

static int sdram_test(void) {
	volatile uint32_t *p = (volatile uint32_t *)MEM_SDRAM;
	uint32_t n = SDRAM_SIZE / 4;
	uint32_t i, err = 0;

	for (i = 0; i < n; i++) {
		p[i] = i | 0xA5A50000u;
	}
	for (i = 0; i < n; i++) {
		if (p[i] != (i | 0xA5A50000u)) {
			err++;
		}
	}

	return (int)err;
}

int main(int argc, char **argv) {
	int err;

	printf("sdram: bringing up 32 MB SDRAM @ 0x%08x\n",
	    (unsigned)MEM_SDRAM);

	if (sdram_init() < 0) {
		printf("sdram: controller did not report INIT_DONE\n");
		return -1;
	}
	printf("sdram: controller initialized\n");

	printf("sdram: testing %lu MB ...\n", (unsigned long)(SDRAM_SIZE >> 20));
	err = sdram_test();

	if (err == 0) {
		printf("sdram: OK (%lu MB)\n", (unsigned long)(SDRAM_SIZE >> 20));
		return 0;
	}

	printf("sdram: FAIL (%u word errors)\n", (unsigned)err);
	return -1;
}
