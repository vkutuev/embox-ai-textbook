/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: USER button increments a counter shown
 *        in binary on the four LEDs (board labels LED1..LED4).
 *        Interrupt-driven variant.
 *
 * The interrupt is armed on the falling edge only (press = high to
 * low). Masking and re-arming write only the GPIOB INTENCLR/INTENSET
 * and INTSTATUS registers directly; they do NOT re-run the driver's
 * gpio_setup_mode(), whose edge-configuration writes re-latch
 * INTSTATUS on this silicon and cause an interrupt storm (see the
 * board guide).
 *
 * Each press: the ISR masks the pin and records the event; the main
 * loop counts, debounces, waits for the release, then clears the
 * status and re-arms. This makes every press count exactly once.
 */

#include <stdio.h>
#include <unistd.h>

#include <drivers/gpio.h>

#include "board.h"

#define GPIOB_BASE      0x40001000UL
#define GPIOB_INTSTATUS (GPIOB_BASE + 0x80)
#define GPIOB_INTENSET  (GPIOB_BASE + 0x60)
#define GPIOB_INTENCLR  (GPIOB_BASE + 0x64)

#define BTN_INT_MASK    BTN_PIN

#define DEBOUNCE_MS     50
#define RELEASE_TIMEOUT 2000

static volatile unsigned int btn_events;

static inline void btn_irq_mask(void) {
	*(volatile unsigned int *)GPIOB_INTENCLR = BTN_INT_MASK;
}

static inline void btn_irq_clear(void) {
	*(volatile unsigned int *)GPIOB_INTSTATUS = BTN_INT_MASK;
}

static inline void btn_irq_arm(void) {
	btn_irq_clear();
	*(volatile unsigned int *)GPIOB_INTENSET = BTN_INT_MASK;
}

static int btn_irq_hnd(unsigned int irq_nr, void *data) {
	btn_events++;
	btn_irq_mask();
	btn_irq_clear();

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

	btn_irq_arm();

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
			unsigned int waited = 0;

			while (waited < RELEASE_TIMEOUT) {
				if (gpio_get(BTN_GPIO_PORT, BTN_PIN) & BTN_PIN) {
					break;
				}
				usleep(10 * 1000);
				waited += 10;
			}
		}

		btn_irq_arm();
	}

	return 0;
}
