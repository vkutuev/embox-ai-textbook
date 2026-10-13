/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: echo bytes received on UART1 (ttyS1).
 *
 * UART1 (0x52011000) is wired to PB5 = TX, PB4 = RX (alt function 3),
 * exposed on the board's XP2 connector (pin 1 = PB4 = RX, pin 3 = PB5 =
 * TX). Requires the pl011_ttyS1 module (see mods.conf.snippet).
 *
 * The example reads from /dev/ttyS1 and writes the same bytes back, so
 * whatever you type on a USB-UART dongle wired to UART1 comes back
 * echoed. The console (ttyS0) stays independent.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define TTY1_DEV "/dev/ttyS1"

int main(int argc, char **argv) {
	char buf[256];
	ssize_t n;
	int fd;

	fd = open(TTY1_DEV, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "uart1_echo: cannot open %s "
				"(is pl011_ttyS1 enabled?)\n", TTY1_DEV);
		return -1;
	}

	printf("uart1_echo: echoing %s (Ctrl+C to stop)\n", TTY1_DEV);

	while (1) {
		n = read(fd, buf, sizeof(buf));
		if (n > 0) {
			write(fd, buf, n);
		}
	}

	return 0;
}
