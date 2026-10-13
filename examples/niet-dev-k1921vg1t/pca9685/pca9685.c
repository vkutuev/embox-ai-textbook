/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: PCA9685 16-channel PWM driver over I2C0.
 *
 * PCA9685 is a 12-bit, 16-channel LED/servo PWM controller (NXP). This
 * example talks to it over I2C0 (bus 0) and lets you configure the PWM
 * frequency and per-channel duty cycle.
 *
 * Wiring (board XP1 / XP3 PLD connectors):
 *   XP1 pin 5 = PD2 = I2C0_SDA  ->  PCA9685 SDA
 *   XP1 pin 3 = PD3 = I2C0_SCL  ->  PCA9685 SCL
 *   XP1 pin 2 (any GND)         ->  PCA9685 GND
 *   XP3 pin 19 (3.3 V)          ->  PCA9685 VCC (logic) and V+ (outputs)
 *
 * The I2C0 bus is already enabled by the k1921vg1t_dev template
 * (embox.driver.i2c.niiet.vg015_i2c0), which configures PD2/PD3 as
 * open-drain with pull-up. Only this command needs to be added.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <drivers/i2c/i2c.h>
#include <drivers/i2c/i2c_bus.h>

#define PCA9685_BUS   0
#define PCA9685_ADDR  0x40

/* Registers */
#define PCA9685_MODE1        0x00
#define PCA9685_MODE2        0x01
#define PCA9685_PRE_SCALE    0xFE
#define PCA9685_LED0_ON_L    0x06
#define PCA9685_ALLLED_ON_L  0xFA
#define PCA9685_ALLLED_OFF_L 0xFC

/* MODE1 bits */
#define MODE1_ALLCALL  0x01
#define MODE1_SLEEP    0x10
#define MODE1_AI       0x20
#define MODE1_EXTCLK   0x40
#define MODE1_RESTART  0x80

/* MODE2 bits */
#define MODE2_OUTDRV   0x04
#define MODE2_INVRT    0x10

#define PCA9685_OSC    25000000UL  /* internal 25 MHz oscillator */

static int pca_read8(uint8_t reg, uint8_t *val) {
	struct i2c_msg msgs[2];

	msgs[0].addr = PCA9685_ADDR;
	msgs[0].flags = 0;
	msgs[0].buf = &reg;
	msgs[0].len = 1;

	msgs[1].addr = PCA9685_ADDR;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = val;
	msgs[1].len = 1;

	return i2c_bus_transfer(PCA9685_BUS, msgs, 2);
}

static int pca_write8(uint8_t reg, uint8_t val) {
	uint8_t buf[2] = { reg, val };

	return i2c_bus_write(PCA9685_BUS, PCA9685_ADDR, buf, 2);
}

static int pca_write_regs(uint8_t reg, const uint8_t *vals, size_t n) {
	uint8_t buf[17];

	if (n > 16) {
		return -EINVAL;
	}

	buf[0] = reg;
	memcpy(&buf[1], vals, n);

	return i2c_bus_write(PCA9685_BUS, PCA9685_ADDR, buf, n + 1);
}

static void usage(void) {
	printf("Usage:\n");
	printf("  pca9685 info\n");
	printf("  pca9685 init [freq_hz]      (default 1000)\n");
	printf("  pca9685 set <channel> <off> (channel 0..15, off 0..4095)\n");
	printf("  pca9685 all <off>           (off 0..4095)\n");
}

static int cmd_info(void) {
	uint8_t mode1, mode2, pre;

	if (pca_read8(PCA9685_MODE1, &mode1) < 0 ||
	    pca_read8(PCA9685_MODE2, &mode2) < 0 ||
	    pca_read8(PCA9685_PRE_SCALE, &pre) < 0) {
		fprintf(stderr, "pca9685: read failed (device at 0x%02x?)\n",
		    PCA9685_ADDR);
		return -1;
	}

	printf("MODE1     = 0x%02x%s%s\n", mode1,
	    (mode1 & MODE1_SLEEP) ? " (sleep)" : "",
	    (mode1 & MODE1_AI) ? " (auto-incr)" : "");
	printf("MODE2     = 0x%02x%s\n", mode2,
	    (mode2 & MODE2_OUTDRV) ? " (totem-pole)" : " (open-drain)");
	printf("PRE_SCALE = %u\n", pre);
	printf("freq      = %lu Hz\n",
	    PCA9685_OSC / (4096UL * (pre + 1)));

	return 0;
}

static int cmd_init(long freq) {
	uint8_t pre;

	if (freq < 24 || freq > 1526) {
		fprintf(stderr, "pca9685: freq out of range (24..1526 Hz)\n");
		return -1;
	}

	pre = (uint8_t)((PCA9685_OSC + 2048UL * (unsigned long)freq) /
	    (4096UL * (unsigned long)freq) - 1);

	/* reset, then sleep to allow PRE_SCALE update */
	if (pca_write8(PCA9685_MODE1, 0x00) < 0 ||
	    pca_write8(PCA9685_MODE1, MODE1_SLEEP) < 0 ||
	    pca_write8(PCA9685_PRE_SCALE, pre) < 0 ||
	    pca_write8(PCA9685_MODE1, MODE1_AI | MODE1_RESTART) < 0 ||
	    pca_write8(PCA9685_MODE2, MODE2_OUTDRV) < 0) {
		fprintf(stderr, "pca9685: init write failed\n");
		return -1;
	}

	printf("pca9685: PRE_SCALE=%u -> ~%lu Hz\n", pre,
	    PCA9685_OSC / (4096UL * (pre + 1)));

	return 0;
}

static int cmd_set(long channel, long off) {
	uint8_t vals[4];

	if (channel < 0 || channel > 15) {
		fprintf(stderr, "pca9685: channel must be 0..15\n");
		return -1;
	}
	if (off < 0 || off > 4096) {
		fprintf(stderr, "pca9685: off must be 0..4096\n");
		return -1;
	}

	/* ON = 0 (start of period), OFF = off. off == 4096 => always off,
	 * off == 0 => always on. */
	vals[0] = 0x00;                 /* LEDn_ON_L */
	vals[1] = 0x00;                 /* LEDn_ON_H */
	vals[2] = off & 0xff;           /* LEDn_OFF_L */
	vals[3] = (off >> 8) & 0x0f;    /* LEDn_OFF_H */

	if (pca_write_regs(PCA9685_LED0_ON_L + 4 * channel, vals, 4) < 0) {
		fprintf(stderr, "pca9685: set failed\n");
		return -1;
	}

	printf("pca9685: channel %ld off=%ld\n", channel, off);

	return 0;
}

static int cmd_all(long off) {
	uint8_t vals[4];

	if (off < 0 || off > 4096) {
		fprintf(stderr, "pca9685: off must be 0..4096\n");
		return -1;
	}

	vals[0] = 0x00;
	vals[1] = 0x00;
	vals[2] = off & 0xff;
	vals[3] = (off >> 8) & 0x0f;

	if (pca_write_regs(PCA9685_ALLLED_ON_L, vals, 4) < 0) {
		fprintf(stderr, "pca9685: set all failed\n");
		return -1;
	}

	printf("pca9685: all channels off=%ld\n", off);

	return 0;
}

int main(int argc, char **argv) {
	long freq = 1000, channel = 0, off = 0;

	if (argc < 2) {
		usage();
		return 0;
	}

	if (strcmp(argv[1], "info") == 0) {
		return cmd_info();
	}

	if (strcmp(argv[1], "init") == 0) {
		if (argc > 2) {
			freq = strtol(argv[2], NULL, 0);
		}
		return cmd_init(freq);
	}

	if (strcmp(argv[1], "set") == 0) {
		if (argc < 4) {
			usage();
			return -1;
		}
		channel = strtol(argv[2], NULL, 0);
		off = strtol(argv[3], NULL, 0);
		return cmd_set(channel, off);
	}

	if (strcmp(argv[1], "all") == 0) {
		if (argc < 3) {
			usage();
			return -1;
		}
		off = strtol(argv[2], NULL, 0);
		return cmd_all(off);
	}

	usage();
	return -1;
}
