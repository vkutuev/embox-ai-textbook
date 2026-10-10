/**
 * @file
 * @brief Shared hardware definitions for the NIIET-DEV-K1921VG1T
 *        LED/button counter examples.
 *
 * Board resources, per the NIIET SDK BSP
 * (hardware/bsp/NIIET-DEV-K1921VG1T/bsp.h):
 *  - LEDs LED0..LED3 on GPIOC pins 12..15, active-low:
 *    clearing a DATAOUT bit lights the LED.
 *  - USER button on GPIOB pin 3.
 *
 * The button polarity is not yet verified on hardware; the examples
 * assume press = low (pull-up + button to ground). If the board wires
 * it the other way, invert the press test in the sources or switch
 * the IRQ edge in led_counter_irq.c.
 */

#ifndef LED_BUTTON_COUNTER_BOARD_H_
#define LED_BUTTON_COUNTER_BOARD_H_

#include <drivers/gpio.h>

#define LED_GPIO_PORT   GPIO_PORT_C
#define LED0_PIN        12
#define LED_PINS        (0xF << LED0_PIN)
#define LED_COUNT       4

#define BTN_GPIO_PORT   GPIO_PORT_B
#define BTN_PIN         (1U << 3)

/*
 * LEDs are active-low. gpio_set() applies one level to a whole pin
 * mask, so write all-off first and then pull the "on" bits low.
 */
static inline void leds_off(void) {
	gpio_set(LED_GPIO_PORT, LED_PINS, GPIO_PIN_HIGH);
}

static inline void leds_write(unsigned int value) {
	unsigned int on = (value & ((1U << LED_COUNT) - 1)) << LED0_PIN;

	leds_off();
	gpio_set(LED_GPIO_PORT, on & LED_PINS, GPIO_PIN_LOW);
}

static inline void leds_init(void) {
	gpio_setup_mode(LED_GPIO_PORT, LED_PINS, GPIO_MODE_OUT);
	leds_off();
}

#endif /* LED_BUTTON_COUNTER_BOARD_H_ */
