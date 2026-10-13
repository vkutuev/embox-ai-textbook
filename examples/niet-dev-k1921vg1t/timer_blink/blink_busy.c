/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: LED1 blinks via a busy-wait delay loop.
 *
 * This is the "wrong" way to time a blink: the CPU spins in a delay
 * loop for the whole period, so the task cannot do anything else and
 * the timing is only as accurate as the loop's calibration. Contrast
 * with blink_tmr, which uses a hardware timer interrupt.
 *
 * DELAY_ITERATIONS is a starting guess for the 60 MHz system clock;
 * measure the actual blink period on hardware and adjust it.
 */

#include <drivers/gpio.h>

#include "board.h"

#define DELAY_ITERATIONS 10000000

static void delay(void) {
	volatile unsigned int i;

	for (i = 0; i < DELAY_ITERATIONS; i++) {
		__asm__ volatile ("nop");
	}
}

int main(int argc, char **argv) {
	led1_init();

	for (;;) {
		led1_toggle();
		delay();
	}

	return 0;
}
