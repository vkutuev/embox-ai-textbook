/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: USER button increments a counter shown
 *        in binary on the four LEDs. Polling variant.
 *
 * Press = pin low is assumed (see board.h). Software debounce: the
 * pin must stay in the new state for DEBOUNCE_POLLS consecutive polls
 * before the state change is accepted.
 */

#include <stdio.h>
#include <unistd.h>

#include <drivers/gpio.h>

#include "board.h"

#define POLL_DELAY_US    (10 * 1000)
#define DEBOUNCE_POLLS   6

int main(int argc, char **argv) {
	unsigned int counter = 0;
	unsigned int stable_cnt = 0;
	int pressed_state = 0;

	leds_init();

	gpio_setup_mode(BTN_GPIO_PORT, BTN_PIN,
			GPIO_MODE_IN | GPIO_MODE_IN_PULL_UP);

	printf("led_counter_poll: press USER to count (Ctrl+C to stop)\n");
	leds_write(counter);

	while (1) {
		int pressed = !(gpio_get(BTN_GPIO_PORT, BTN_PIN) & BTN_PIN);

		usleep(POLL_DELAY_US);

		if (pressed == pressed_state) {
			stable_cnt = 0;
			continue;
		}

		if (++stable_cnt < DEBOUNCE_POLLS) {
			continue;
		}

		pressed_state = pressed;
		stable_cnt = 0;

		if (pressed) {
			counter = (counter + 1) & ((1U << LED_COUNT) - 1);
			leds_write(counter);
			printf("counter = %u\n", counter);
		}
	}

	return 0;
}
