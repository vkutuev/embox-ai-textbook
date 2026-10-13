/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: LED1 blinks at exactly 1 Hz using the
 *        TMR1 hardware timer and its interrupt.
 *
 * TMR1 (0x51009000) is used because TMR0 is the Embox jiffies clock
 * source. The timer runs at the 189 MHz clock (27 MHz HSE x 7,
 * measured on hardware); PERIOD is set so the period interrupt fires
 * at 2 Hz, and the handler toggles LED1 (one full on/off cycle per
 * second).
 *
 * The timer interrupt path (PLIC source 17) is safe: it is the same
 * mechanism Embox uses for its own clock tick (TMR0, source 16), so
 * unlike the GPIO interrupt there is no latch quirk here.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <drivers/gpio.h>
#include <kernel/irq.h>

#include "board.h"

#define RCU_BASE        0x50004000UL
#define RCU_CGCFGAPB1   (RCU_BASE + 0x008)
#define RCU_RSTDISAPB1  (RCU_BASE + 0x018)

#define SIU_BASE        0x50003000UL
#define SIU_TMREN       (SIU_BASE + 0x54)

#define TMR1_BASE       0x51009000UL
#define TMR_CTRL        (TMR1_BASE + 0x00)
#define TMR_PERIOD      (TMR1_BASE + 0x0C)
#define TMR_IM          (TMR1_BASE + 0x10)
#define TMR_IC          (TMR1_BASE + 0x1C)

#define TMR1_CLK_BIT    (1U << 17)      /* RCU CGCFGAPB1 / RSTDISAPB1 */
#define TMR1_EN_BIT     (1U << 1)       /* SIU TMREN */

#define TMR_CTRL_MODE_1 (1U << 4)       /* count 0..PERIOD, IRQ at period */
#define TMR_IM_TMR      (1U << 0)       /* period interrupt mask */

#define TMR_CLK_HZ      189000000UL   /* measured: 27 MHz HSE x 7 (~189 MHz) */
#define BLINK_HZ        1UL
#define TOGGLE_HZ       (2UL * BLINK_HZ)
#define TMR_PERIOD_VAL  (TMR_CLK_HZ / TOGGLE_HZ - 1UL)

#define TMR1_IRQ        17

static volatile unsigned int ticks;
static volatile int running = 1;

static irq_return_t tmr1_irq_hnd(unsigned int irq_nr, void *data) {
	*(volatile unsigned int *)TMR_IC = TMR_IM_TMR;
	ticks++;
	led1_toggle();

	return IRQ_HANDLED;
}

static void sigint_handler(int sig) {
	running = 0;
}

int main(int argc, char **argv) {
	led1_init();

	*(volatile unsigned int *)RCU_CGCFGAPB1  |= TMR1_CLK_BIT;
	*(volatile unsigned int *)RCU_RSTDISAPB1 |= TMR1_CLK_BIT;

	*(volatile unsigned int *)TMR_CTRL   = 0;
	*(volatile unsigned int *)TMR_PERIOD = TMR_PERIOD_VAL;
	*(volatile unsigned int *)TMR_IM     = TMR_IM_TMR;

	/* Drop a stale registration from a previous run of this command. */
	irq_detach(TMR1_IRQ, NULL);

	if (0 > irq_attach(TMR1_IRQ, tmr1_irq_hnd, 0, NULL, "TMR1")) {
		fprintf(stderr, "blink_tmr: cannot attach TMR1 IRQ\n");
		return -1;
	}

	*(volatile unsigned int *)TMR_CTRL |= TMR_CTRL_MODE_1;
	*(volatile unsigned int *)SIU_TMREN |= TMR1_EN_BIT;

	signal(SIGINT, sigint_handler);

	printf("blink_tmr: LED1 blinking at %lu Hz (Ctrl+C to stop)\n",
			(unsigned long)BLINK_HZ);

	while (running) {
		usleep(100 * 1000);
	}

	irq_detach(TMR1_IRQ, NULL);
	*(volatile unsigned int *)TMR_IM = 0;
	*(volatile unsigned int *)TMR_CTRL = 0;
	*(volatile unsigned int *)SIU_TMREN &= ~TMR1_EN_BIT;

	printf("blink_tmr: stopped\n");

	return 0;
}
