/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: Sharp GP2Y0A21YK0F IR distance sensor.
 *
 * Reads the sensor's analog output on PA8 (ADC channel 8, block 0) and
 * converts it to distance. The sensor is a 5 V device powered from the
 * board's 5 V rail; its output is ~3.1 V at 10 cm and ~0.4 V at 80 cm,
 * within the 3.3 V ADC range (keep objects >= 10 cm).
 *
 * Embox has no ADC driver, so the command programs the ADC directly
 * (RCU ADCCFG, ACTL[0], sequencer 0), like the tsensor example.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* RCU */
#define RCU_CGCFGAHB    0x50004000UL
#define RCU_RSTDISAHB   0x50004010UL
#define RCU_ADCCFG      0x50004090UL

/* ADC */
#define ADC_BASE        0x41020000UL
#define ADC_SEQEN       (ADC_BASE + 0x00)
#define ADC_SEQSYNC     (ADC_BASE + 0x04)
#define ADC_BSTAT       (ADC_BASE + 0x0C)
#define ADC_EMUX        (ADC_BASE + 0x1C)
#define ADC_RIS         (ADC_BASE + 0x20)
#define ADC_IC          (ADC_BASE + 0x2C)
#define ADC_SEQ0        (ADC_BASE + 0x40)
#define ADC_SEQ0_SRQSEL0 (ADC_SEQ0 + 0x00)
#define ADC_SEQ0_SRQCTL  (ADC_SEQ0 + 0x18)
#define ADC_SEQ0_SCCTL   (ADC_SEQ0 + 0x24)
#define ADC_SEQ0_SRTMR   (ADC_SEQ0 + 0x30)
#define ADC_SEQ0_SFLOAD  (ADC_SEQ0 + 0x34)
#define ADC_SEQ0_SFIFO   (ADC_SEQ0 + 0x38)
#define ADC_ACTL0        (ADC_BASE + 0x540)
#define ADC_CHDELAY8     (ADC_BASE + 0x680 + 8 * 4)

/* GPIO A (clear output/alt function on PA8 so the ADC can sample it) */
#define GPIOA_OUTENCLR   0x40000030UL
#define GPIOA_ALTFUNCCLR 0x40000038UL

#define ADCCFG_CLKEN      (1u << 0)
#define ADCCFG_RSTDIS     (1u << 4)
#define ADCCFG_CLKSEL(v)  ((v) << 8)     /* 1 = HSE */
#define RCU_AHB_ADCEN     (1u << 10)

#define ACTL_ADCEN        (1u << 0)
#define ACTL_ADCRDY       (1u << 1)
#define ACTL_ADCENLDO     (1u << 2)
#define ACTL_ADCSTART     (1u << 3)
#define ACTL_SELRES_12    (3u << 4)
#define ACTL_RANGELDO     (1u << 12)

#define ADC_SEQEN_SEQEN0  (1u << 0)
#define ADC_SEQSYNC_SYNC0 (1u << 0)
#define ADC_SEQSYNC_GSYNC (1u << 31)
#define ADC_RIS_SEQRIS0   (1u << 0)

#define SHARP_CH          8
#define HSECLK_HZ         27000000UL
#define AVCC_MV           3300UL

#define RD(reg)           (*(volatile uint32_t *)(reg))
#define WR(reg, val)      (*(volatile uint32_t *)(reg) = (val))

static void settle_delay(void) {
	volatile unsigned int i;

	for (i = 0; i < 5000; i++) {
		__asm__ __volatile__("nop");
	}
}

static int adc_init(void) {
	volatile uint32_t timeout;

	/* make PA8 a hi-Z input (no output driver, no alt function) */
	WR(GPIOA_OUTENCLR, 1u << SHARP_CH);
	WR(GPIOA_ALTFUNCCLR, 1u << SHARP_CH);

	/* ADC clock from HSE, release reset */
	WR(RCU_ADCCFG, RD(RCU_ADCCFG) & ~ADCCFG_RSTDIS);
	WR(RCU_RSTDISAHB, RD(RCU_RSTDISAHB) & ~RCU_AHB_ADCEN);
	WR(RCU_ADCCFG, ADCCFG_CLKSEL(1) | ADCCFG_CLKEN);
	WR(RCU_ADCCFG, RD(RCU_ADCCFG) | ADCCFG_RSTDIS);
	WR(RCU_CGCFGAHB, RD(RCU_CGCFGAHB) | RCU_AHB_ADCEN);
	WR(RCU_RSTDISAHB, RD(RCU_RSTDISAHB) | RCU_AHB_ADCEN);

	/* ADC block 0 (channels 0-15): LDO + 12-bit + enable */
	WR(ADC_ACTL0, ACTL_ADCENLDO | ACTL_RANGELDO | ACTL_SELRES_12);
	WR(ADC_ACTL0, RD(ADC_ACTL0) | ACTL_ADCEN);

	settle_delay();

	WR(ADC_ACTL0, RD(ADC_ACTL0) | ACTL_ADCSTART);

	timeout = 0;
	while (!(RD(ADC_ACTL0) & ACTL_ADCRDY)) {
		if (++timeout > 2000000) {
			return -1;
		}
	}

	/* sequencer 0: software trigger, 3 samples of channel 8 */
	WR(ADC_EMUX, 0);
	WR(ADC_SEQ0_SCCTL, 0);
	WR(ADC_SEQ0_SRTMR, 0);
	WR(ADC_SEQ0_SRQCTL, 2);                         /* RQMAX=2 (3 samples) */
	WR(ADC_SEQ0_SRQSEL0,
	    SHARP_CH | (SHARP_CH << 8) | (SHARP_CH << 16));
	WR(ADC_CHDELAY8, HSECLK_HZ / 10000);

	WR(ADC_SEQSYNC, ADC_SEQSYNC_SYNC0);
	WR(ADC_SEQEN, ADC_SEQEN_SEQEN0);

	return 0;
}

static uint16_t adc_read_sharp(void) {
	uint32_t s[3];
	int i;

	WR(ADC_IC, ADC_RIS_SEQRIS0);
	/* keep SYNC0 set, pulse GSYNC to trigger */
	WR(ADC_SEQSYNC, ADC_SEQSYNC_SYNC0 | ADC_SEQSYNC_GSYNC);

	while (RD(ADC_BSTAT)) {
	}
	while (!(RD(ADC_RIS) & ADC_RIS_SEQRIS0)) {
	}
	while (RD(ADC_SEQ0_SFLOAD) < 3) {
	}

	for (i = 0; i < 3; i++) {
		s[i] = RD(ADC_SEQ0_SFIFO) & 0xFFF;
	}

	return (uint16_t)((s[0] + s[1] + s[2]) / 3);
}

int main(int argc, char **argv) {
	long n = 1, i;

	if (argc > 1) {
		n = strtol(argv[1], NULL, 0);
	}
	if (n < 1) {
		n = 1;
	}

	if (adc_init() < 0) {
		fprintf(stderr, "sharp: ADC did not become ready\n");
		return -1;
	}

	printf("sharp: PA8 (ADC ch%d), %ld sample%s\n",
	    SHARP_CH, n, n == 1 ? "" : "s");

	for (i = 0; i < n; i++) {
		uint16_t raw = adc_read_sharp();
		uint32_t mv = (uint32_t)raw * AVCC_MV / 4096;
		int32_t d;

		if (mv < 100) {
			printf("[%ld] raw=%u  V=%lu mV  d=>80 cm\n",
			    i + 1, (unsigned)raw, (unsigned long)mv);
		} else {
			/* d = 30.86 / (V - 0.014), V in volts */
			d = 30860 / (int32_t)(mv - 14);
			printf("[%ld] raw=%u  V=%lu mV  d=%ld cm\n",
			    i + 1, (unsigned)raw, (unsigned long)mv, (long)d);
		}

		if (i + 1 < n) {
			usleep(500 * 1000);
		}
	}

	return 0;
}
