/**
 * @file
 * @brief Shared board definitions for the NIIET-DEV-K1921VG1T timer
 *        blink examples.
 *
 * LED1 on GPIOC pin 12, active-high (pin high = LED on) — verified on
 * hardware in the LED/button counter example.
 */

#ifndef TIMER_BLINK_BOARD_H_
#define TIMER_BLINK_BOARD_H_

#include <drivers/gpio.h>

#define LED_GPIO_PORT   GPIO_PORT_C
#define LED1_PIN        12
#define LED1_MSK        (1U << LED1_PIN)

static inline void led1_init(void) {
	gpio_setup_mode(LED_GPIO_PORT, LED1_MSK, GPIO_MODE_OUT);
	gpio_set(LED_GPIO_PORT, LED1_MSK, GPIO_PIN_LOW);
}

static inline void led1_toggle(void) {
	gpio_toggle(LED_GPIO_PORT, LED1_MSK);
}

#endif /* TIMER_BLINK_BOARD_H_ */
