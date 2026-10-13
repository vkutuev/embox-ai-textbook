/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: PWM output via the niiet PWM driver.
 *
 * The VG1T generates PWM with its timer units (TMR) in PWM mode. The
 * k1921vg1t_dev template already enables `niiet_pwm1` (TMR1), whose
 * OUT0..OUT3 map to PA4..PA7 (alt function 5), exposed on XP1 pins
 * 19/21/23/25.
 *
 * This command uses the PWM core API: pwm_dev_by_id() to find the
 * device, then pwm_set_frequency(), pwm_set_duty() (duty in ns) and
 * pwm_enable() (bitmask of channels).
 *
 * Note: the driver computes timing from SYSTEM_FREQ = 150 MHz, but the
 * real TMR clock is ~189 MHz (27 MHz HSE x 7), so the actual output
 * frequency is 1.26x the requested value. The duty *percentage* is
 * preserved (period and duty scale together).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <drivers/pwm.h>

#define PWM_ID    1    /* niiet_pwm1 (TMR1) */
#define PWM_CHAN  0    /* OUT0 = PA4 (XP1 pin 19) */

static int set_pwm(int freq_hz, int duty_pct) {
	struct pwm_device *pwm;
	int period_ns, duty_ns;

	pwm = pwm_dev_by_id(PWM_ID);
	if (pwm == NULL) {
		fprintf(stderr, "pwm_demo: no PWM%d (is niiet_pwm1 enabled?)\n",
		    PWM_ID);
		return -1;
	}
	if (freq_hz < 1 || duty_pct < 0 || duty_pct > 100) {
		fprintf(stderr, "pwm_demo: bad arguments\n");
		return -1;
	}

	period_ns = 1000000000 / freq_hz;
	duty_ns = (int)((long long)period_ns * duty_pct / 100);
	if (duty_ns < 2) {
		duty_ns = 2;   /* pwm_set_duty rejects < 2 ns */
	}

	if (pwm_set_frequency(pwm, freq_hz) != 0 ||
	    pwm_set_duty(pwm, PWM_CHAN, duty_ns) != 0 ||
	    pwm_enable(pwm, 1U << PWM_CHAN) != 0) {
		fprintf(stderr, "pwm_demo: PWM setup failed\n");
		return -1;
	}

	printf("pwm_demo: PWM%d chan%d freq=%d Hz duty=%d%%\n",
	    PWM_ID, PWM_CHAN, freq_hz, duty_pct);

	return 0;
}

static void usage(void) {
	printf("Usage:\n");
	printf("  pwm_demo <freq_hz> <duty_pct>   static output (duty 0..100)\n");
	printf("  pwm_demo sweep <freq_hz>        duty sweep 0%%..100%%..0%%\n");
}

int main(int argc, char **argv) {
	int freq, i;

	if (argc == 3 && strcmp(argv[1], "sweep") != 0) {
		return set_pwm(atoi(argv[1]), atoi(argv[2]));
	}

	if (argc == 3 && strcmp(argv[1], "sweep") == 0) {
		freq = atoi(argv[2]);

		/* 5% steps, 30 ms each: 0% -> 100% -> 0% */
		for (i = 0; i <= 100; i += 5) {
			set_pwm(freq, i);
			usleep(30 * 1000);
		}
		for (i = 95; i >= 0; i -= 5) {
			set_pwm(freq, i);
			usleep(30 * 1000);
		}
		set_pwm(freq, 50);

		return 0;
	}

	usage();
	return -1;
}
