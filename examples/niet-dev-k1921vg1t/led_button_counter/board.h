/**
 * @file
 * @brief Shared hardware definitions for the NIIET-DEV-K1921VG1T
 *        LED/button counter examples.
 *
 * Board resources, per the NIIET SDK BSP
 * (hardware/bsp/NIIET-DEV-K1921VG1T/bsp.h), verified on hardware:
 *  - LEDs LED1..LED4 (board silkscreen) on GPIOC pins 12..15,
 *    active-high: driving a pin high lights the LED. (The SDK BSP code
 *    implies the opposite — BSP_LED_On() clears the output bit — but
 *    active-high was confirmed empirically on this board.)
 *  - USER button on GPIOB pin 3, active-low: the pin reads high when
 *    released and low while pressed.
 *
 * All examples assume press = low (pull-up + button to ground).
 */

#ifndef LED_BUTTON_COUNTER_BOARD_H_
#define LED_BUTTON_COUNTER_BOARD_H_

#include <drivers/gpio.h>

#define LED_GPIO_PORT   GPIO_PORT_C
#define LED1_PIN        12
#define LED_PINS        (0xF << LED1_PIN)
#define LED_COUNT       4

#define BTN_GPIO_PORT   GPIO_PORT_B
#define BTN_PIN         (1U << 3)

/*
 * LEDs on this board are active-high. gpio_set() applies one level to
 * a whole pin mask, so write all-off first and then pull the "on"
 * bits high.
 */
static inline void leds_off(void) {
	gpio_set(LED_GPIO_PORT, LED_PINS, GPIO_PIN_LOW);
}

static inline void leds_write(unsigned int value) {
	unsigned int on = (value & ((1U << LED_COUNT) - 1)) << LED1_PIN;

	leds_off();
	gpio_set(LED_GPIO_PORT, on & LED_PINS, GPIO_PIN_HIGH);
}

static inline void leds_init(void) {
	gpio_setup_mode(LED_GPIO_PORT, LED_PINS, GPIO_MODE_OUT);
	leds_off();
}

#endif /* LED_BUTTON_COUNTER_BOARD_H_ */
