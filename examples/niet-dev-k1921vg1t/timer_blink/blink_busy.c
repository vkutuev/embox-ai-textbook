/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: LED1 blinks via a busy-wait delay loop.
 *
 * The loop cost is calibrated once at startup: TMR1's free-running
 * counter is measured against usleep() (a known real-time interval)
 * and against the delay loop itself, so the iteration count for a
 * 500 ms half-period is computed without hard-coding the CPU clock.
 * The CPU still spins in the delay loop the whole time — contrast
 * with blink_tmr, which uses a hardware timer interrupt.
 */

#include <stdio.h>
#include <unistd.h>

#include <drivers/gpio.h>

#include "board.h"

#define RCU_BASE        0x50004000UL
#define RCU_CGCFGAPB1   (RCU_BASE + 0x008)
#define RCU_RSTDISAPB1  (RCU_BASE + 0x018)

#define SIU_BASE        0x50003000UL
#define SIU_TMREN       (SIU_BASE + 0x54)

#define TMR1_BASE       0x51009000UL
#define TMR_CTRL        (TMR1_BASE + 0x00)
#define TMR_COUNT       (TMR1_BASE + 0x04)
#define TMR_PERIOD      (TMR1_BASE + 0x0C)
#define TMR_IM          (TMR1_BASE + 0x10)

#define TMR1_CLK_BIT    (1U << 17)      /* RCU CGCFGAPB1 / RSTDISAPB1 */
#define TMR1_EN_BIT     (1U << 1)       /* SIU TMREN */

#define TMR_CTRL_CLR    (1U << 2)       /* clear counter */
#define TMR_CTRL_MODE_1 (1U << 4)       /* MODE=Up (count up) */

#define REF_ITERS       1000000UL

static inline unsigned int tmr1_count(void) {
	return *(volatile unsigned int *)TMR_COUNT;
}

static void delay(unsigned int iterations) {
	volatile unsigned int i;

	for (i = 0; i < iterations; i++) {
		__asm__ volatile ("nop");
	}
}

int main(int argc, char **argv) {
	unsigned int t0, t1, s0, s1;
	unsigned int tmr_per_ms, tmr_per_iter, iterations;

	led1_init();

	*(volatile unsigned int *)RCU_CGCFGAPB1  |= TMR1_CLK_BIT;
	*(volatile unsigned int *)RCU_RSTDISAPB1 |= TMR1_CLK_BIT;
	*(volatile unsigned int *)TMR_CTRL        = 0;
	*(volatile unsigned int *)TMR_PERIOD      = 0xFFFFFFFFUL;
	*(volatile unsigned int *)TMR_IM          = 0;
	*(volatile unsigned int *)TMR_CTRL        = TMR_CTRL_CLR | TMR_CTRL_MODE_1;
	*(volatile unsigned int *)SIU_TMREN      |= TMR1_EN_BIT;

	t0 = tmr1_count();
	usleep(200000);
	t1 = tmr1_count();
	tmr_per_ms = (t1 - t0) / 200;

	s0 = tmr1_count();
	delay(REF_ITERS);
	s1 = tmr1_count();
	tmr_per_iter = (s1 - s0) / REF_ITERS;

	if (tmr_per_iter == 0) {
		iterations = REF_ITERS;
	} else {
		iterations = tmr_per_ms * 500 / tmr_per_iter;
	}

	printf("blink_busy: %u iterations per 500 ms\n", iterations);

	for (;;) {
		led1_toggle();
		delay(iterations);
	}

	return 0;
}
