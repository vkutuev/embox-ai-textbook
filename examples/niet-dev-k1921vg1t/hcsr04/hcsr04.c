/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: HC-SR04 ultrasonic distance sensor.
 *
 * Drives an HC-SR04 over two GPIO lines and times the ECHO pulse with
 * the TMR1 free-running counter (189 MHz, 27 MHz HSE x 7).
 *
 * Wiring (sensor powered from 3.3 V so its ECHO is MCU-safe):
 *   HC-SR04 VCC  -> XP3 pin 19 (3.3 V)
 *   HC-SR04 GND  -> XP1 pin 2  (any GND)
 *   HC-SR04 TRIG -> XP1 pin 9  (PD0, output)
 *   HC-SR04 ECHO -> XP1 pin 7  (PD1, input)
 *
 * Note: the HC-SR04 is specified for 4.5-5.5 V. At 3.3 V it runs out of
 * spec with reduced range, but its ECHO then swings 0-3.3 V, which is
 * safe for the (non-5V-tolerant) MCU pins. No level shifter needed.
 *
 * Distance: d[cm] = echo_us / 58 (speed of sound ~343 m/s, round trip).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <drivers/gpio.h>

/* TMR1 (free-running counter for echo timing) */
#define TMR1_BASE       0x51009000UL
#define TMR_CTRL        (TMR1_BASE + 0x00)
#define TMR_COUNT       (TMR1_BASE + 0x04)
#define TMR_CLKDIV      (TMR1_BASE + 0x08)
#define TMR_PERIOD      (TMR1_BASE + 0x0C)

#define RCU_BASE        0x50004000UL
#define RCU_CGCFGAPB1   (RCU_BASE + 0x008)
#define RCU_RSTDISAPB1  (RCU_BASE + 0x018)

#define SIU_BASE        0x50003000UL
#define SIU_TMREN       (SIU_BASE + 0x54)

#define TMR1_CLK_BIT    (1U << 17)      /* RCU CGCFGAPB1 / RSTDISAPB1 */
#define TMR1_EN_BIT     (1U << 1)       /* SIU TMREN */

#define TMR_CTRL_MODE_1 (1U << 4)       /* count 0..PERIOD, wrap at period */

/* Pins */
#define TRIG_PORT       GPIO_PORT_D
#define TRIG_PIN        0
#define ECHO_PORT       GPIO_PORT_D
#define ECHO_PIN        1
#define TRIG_MSK        (1U << TRIG_PIN)
#define ECHO_MSK        (1U << ECHO_PIN)

#define TMR_TICKS_PER_US 189UL          /* 189 MHz */

#define TRIG_US         20UL            /* trigger pulse width (>=10 us) */
#define ECHO_STUCK_US   40000UL         /* ECHO high longer than this => stuck */

static uint32_t tmr_count(void) {
	return *(volatile uint32_t *)TMR_COUNT;
}

static void tmr_delay_us(uint32_t us) {
	uint32_t start = tmr_count();
	uint32_t ticks = us * TMR_TICKS_PER_US;

	while ((uint32_t)(tmr_count() - start) < ticks) {
	}
}

static void tmr1_init(void) {
	*(volatile uint32_t *)RCU_CGCFGAPB1  |= TMR1_CLK_BIT;
	*(volatile uint32_t *)RCU_RSTDISAPB1 |= TMR1_CLK_BIT;

	*(volatile uint32_t *)TMR_CTRL   = 0;
	*(volatile uint32_t *)TMR_CLKDIV = 0;          /* full 189 MHz */
	*(volatile uint32_t *)TMR_PERIOD = 0xffffffff; /* effectively free-run */
	*(volatile uint32_t *)TMR_CTRL   = TMR_CTRL_MODE_1;
	*(volatile uint32_t *)SIU_TMREN  |= TMR1_EN_BIT;
}

/* Trigger a measurement and return the ECHO high time in us.
 * Returns 0 on success, -1 if ECHO never rose, -2 if ECHO stuck high. */
static int measure(uint32_t *us_out) {
	uint32_t t0, t1;

	gpio_set(TRIG_PORT, TRIG_MSK, GPIO_PIN_HIGH);
	tmr_delay_us(TRIG_US);
	gpio_set(TRIG_PORT, TRIG_MSK, GPIO_PIN_LOW);

	t0 = tmr_count();

	/* wait for ECHO rising edge (timeout 100 ms) */
	while (!(gpio_get(ECHO_PORT, ECHO_MSK))) {
		if ((uint32_t)(tmr_count() - t0) > 100UL * TMR_TICKS_PER_US) {
			return -1;
		}
	}
	t1 = tmr_count();

	/* wait for ECHO falling edge (timeout ECHO_STUCK_US) */
	while (gpio_get(ECHO_PORT, ECHO_MSK)) {
		if ((uint32_t)(tmr_count() - t1) > ECHO_STUCK_US * TMR_TICKS_PER_US) {
			return -2;
		}
	}

	*us_out = (uint32_t)(tmr_count() - t1) / TMR_TICKS_PER_US;
	return 0;
}

int main(int argc, char **argv) {
	long n = 1, i;

	if (argc > 1) {
		n = strtol(argv[1], NULL, 0);
	}
	if (n < 1) {
		n = 1;
	}

	gpio_setup_mode(TRIG_PORT, TRIG_MSK, GPIO_MODE_OUT);
	gpio_setup_mode(ECHO_PORT, ECHO_MSK, GPIO_MODE_IN);
	gpio_set(TRIG_PORT, TRIG_MSK, GPIO_PIN_LOW);

	tmr1_init();

	printf("hcsr04: TRIG=PD0 ECHO=PD1 (%ld sample%s)\n",
	    n, n == 1 ? "" : "s");

	for (i = 0; i < n; i++) {
		uint32_t us = 0;
		int r = measure(&us);

		if (r == -1) {
			printf("[%ld] no echo (ECHO stayed low)\n", i + 1);
		} else if (r == -2) {
			printf("[%ld] no echo (ECHO stuck high >40 ms)\n", i + 1);
		} else if (us > 25000) {
			printf("[%ld] out of range (echo=%lu us)\n", i + 1,
			    (unsigned long)us);
		} else {
			printf("[%ld] echo=%lu us, dist=%lu cm\n", i + 1,
			    (unsigned long)us, (unsigned long)(us / 58));
		}

		if (i + 1 < n) {
			usleep(500 * 1000);   /* HC-SR04 min cycle ~60 ms */
		}
	}

	return 0;
}
