/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: USER button increments a counter shown
 *        in binary on the four LEDs. Interrupt-driven variant.
 *
 * The interrupt is armed on the falling edge only (press = high to
 * low). A both-edges configuration would latch INTSTATUS while the
 * button idles high and storm the PLIC (see the board guide).
 *
 * To survive contact bounce, the handler masks the pin interrupt
 * immediately; the main loop debounces, waits for the release, and
 * only then re-arms the interrupt. This makes each press count
 * exactly once and prevents interrupt storms.
 */

#include <stdio.h>
#include <unistd.h>

#include <drivers/gpio.h>

#include "board.h"

#define DEBOUNCE_MS      50
#define RELEASE_TIMEOUT  2000

static volatile unsigned int btn_events;

static int btn_irq_hnd(unsigned int irq_nr, void *data) {
	btn_events++;
	gpio_setup_mode(BTN_GPIO_PORT, BTN_PIN, GPIO_MODE_INT_SECTION);

	return 0;
}

int main(int argc, char **argv) {
	unsigned int counter = 0;
	unsigned int events = 0;

	leds_init();

	gpio_setup_mode(BTN_GPIO_PORT, BTN_PIN,
			GPIO_MODE_IN | GPIO_MODE_IN_PULL_UP |
			GPIO_MODE_INT_FALLING);

	if (0 > gpio_irq_attach(BTN_GPIO_PORT, BTN_PIN, btn_irq_hnd, NULL)) {
		fprintf(stderr, "led_counter_irq: cannot attach GPIO IRQ\n");
		return -1;
	}

	gpio_setup_mode(BTN_GPIO_PORT, BTN_PIN,
			GPIO_MODE_INT_EN | GPIO_MODE_INT_FALLING);

	printf("led_counter_irq: press USER to count (Ctrl+C to stop)\n");
	leds_write(counter);

	while (1) {
		if (btn_events == events) {
			usleep(10 * 1000);
			continue;
		}

		events = btn_events;
		counter = (counter + 1) & ((1U << LED_COUNT) - 1);
		leds_write(counter);
		printf("counter = %u\n", counter);

		usleep(DEBOUNCE_MS * 1000);

		{
			int released = 0;
			unsigned int waited = 0;

			while (waited < RELEASE_TIMEOUT) {
				if (gpio_get(BTN_GPIO_PORT, BTN_PIN) & BTN_PIN) {
					released = 1;
					break;
				}
				usleep(10 * 1000);
				waited += 10;
			}

			if (!released) {
				printf("still pressed, waiting for release\n");
				continue;
			}
		}

		gpio_setup_mode(BTN_GPIO_PORT, BTN_PIN,
				GPIO_MODE_INT_EN | GPIO_MODE_INT_FALLING);
	}

	return 0;
}
