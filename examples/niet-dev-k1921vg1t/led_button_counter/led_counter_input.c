/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: USER button increments a counter shown
 *        in binary on the four LEDs. Input-subsystem variant.
 *
 * Button events are produced by the gpio_button input driver and read
 * from /dev/input/user-bt. Enable the driver in mods.conf:
 *
 *   include embox.driver.input.button.gpio_button(pin_port=1, pin_num=3)
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <drivers/input/input_dev.h>

#include "board.h"

#define BTN_DEV "/dev/input/user-bt"

int main(int argc, char **argv) {
	struct input_event ev;
	unsigned int counter = 0;
	int fd;

	fd = open(BTN_DEV, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr,
				"led_counter_input: cannot open %s.\n"
				"Is the gpio_button driver enabled? "
				"(see mods.conf.snippet)\n",
				BTN_DEV);
		return -1;
	}

	leds_init();

	printf("led_counter_input: press USER to count (Ctrl+C to stop)\n");
	leds_write(counter);

	while (1) {
		if (read(fd, &ev, sizeof(ev)) <= 0) {
			continue;
		}

		if (ev.type == USER_BUTTON_PRESSED) {
			counter = (counter + 1) & ((1U << LED_COUNT) - 1);
			leds_write(counter);
			printf("counter = %u\n", counter);
		}
	}

	return 0;
}
