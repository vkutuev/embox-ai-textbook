/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: USER button increments a counter shown
 *        in binary on the four LEDs. Interrupt-driven variant.
 *
 * A GPIO interrupt is armed on both edges of the button pin; the
 * handler counts only the edge into the pressed (low) state, which
 * filters most switch bounce. The main loop applies the events in
 * 50 ms windows, which also acts as a simple debounce.
 */

#include <stdio.h>
#include <unistd.h>

#include <drivers/gpio.h>

#include "board.h"

static volatile unsigned int btn_events;

static int btn_irq_hnd(unsigned int irq_nr, void *data) {
	if (!(gpio_get(BTN_GPIO_PORT, BTN_PIN) & BTN_PIN)) {
		btn_events++;
	}

	return 0;
}

int main(int argc, char **argv) {
	unsigned int counter = 0;
	unsigned int events = 0;

	leds_init();

	gpio_setup_mode(BTN_GPIO_PORT, BTN_PIN,
			GPIO_MODE_IN | GPIO_MODE_IN_PULL_UP |
			GPIO_MODE_INT_EN | GPIO_MODE_INT_RISING_FALLING);

	if (0 > gpio_irq_attach(BTN_GPIO_PORT, BTN_PIN, btn_irq_hnd, NULL)) {
		fprintf(stderr, "led_counter_irq: cannot attach GPIO IRQ\n");
		return -1;
	}

	printf("led_counter_irq: press USER to count (Ctrl+C to stop)\n");
	leds_write(counter);

	while (1) {
		usleep(50 * 1000);

		if (btn_events != events) {
			events = btn_events;
			counter = (counter + 1) & ((1U << LED_COUNT) - 1);
			leds_write(counter);
			printf("counter = %u\n", counter);
		}
	}

	return 0;
}
