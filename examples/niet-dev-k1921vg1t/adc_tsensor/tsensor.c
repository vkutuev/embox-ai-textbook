/**
 * @file
 * @brief NIIET-DEV-K1921VG1T: internal temperature sensor via the ADC.
 *
 * The VG1T has a 12-bit ADC (0x41020000) with 4 blocks and 64 channels.
 * The internal temperature sensor is channel 63 on ADC block 3. Embox has
 * no ADC driver yet, so this example programs the ADC registers directly
 * (RCU clock, ACTL[3], sequencer 0, SIU TSENSCTRL).
 *
 * Temperature: V = (raw/4096) * 3.3 V;  T = 574.713 * (0.793 - V)  (C)
 * for the scaled sensor output (SIU TSENSCTRL.SCALE = 1).
 *
 * No wiring needed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* RCU */
#define RCU_CGCFGAHB    0x50004000UL
#define RCU_RSTDISAHB   0x50004010UL
#define RCU_ADCCFG      0x50004090UL

/* SIU */
#define SIU_TSENSCTRL   0x5000304CUL

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
#define ADC_ACTL3        (ADC_BASE + 0x570)
#define ADC_CHDELAY63    (ADC_BASE + 0x77C)

/* RCU ADCCFG bits */
#define ADCCFG_CLKEN      (1u << 0)
#define ADCCFG_RSTDIS     (1u << 4)
#define ADCCFG_CLKSEL(v)  ((v) << 8)     /* 1 = HSE */
#define ADCCFG_DIVEN      (1u << 12)
#define RCU_AHB_ADCEN     (1u << 10)

/* SIU TSENSCTRL bits */
#define TSENSCTRL_DIS     (1u << 0)
#define TSENSCTRL_SCALE   (1u << 1)

/* ADC ACTL bits */
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

#define TSENSOR_CH        63
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

	/* hold the ADC in reset, select HSE clock, release */
	WR(RCU_ADCCFG, RD(RCU_ADCCFG) & ~ADCCFG_RSTDIS);
	WR(RCU_RSTDISAHB, RD(RCU_RSTDISAHB) & ~RCU_AHB_ADCEN);

	WR(RCU_ADCCFG, ADCCFG_CLKSEL(1) | ADCCFG_CLKEN);
	WR(RCU_ADCCFG, RD(RCU_ADCCFG) | ADCCFG_RSTDIS);
	WR(RCU_CGCFGAHB, RD(RCU_CGCFGAHB) | RCU_AHB_ADCEN);
	WR(RCU_RSTDISAHB, RD(RCU_RSTDISAHB) | RCU_AHB_ADCEN);

	/* ADC block 3 (temperature sensor): LDO + 12-bit + enable */
	WR(ADC_ACTL3, ACTL_ADCENLDO | ACTL_RANGELDO | ACTL_SELRES_12);
	WR(ADC_ACTL3, RD(ADC_ACTL3) | ACTL_ADCEN);

	settle_delay();

	WR(ADC_ACTL3, RD(ADC_ACTL3) | ACTL_ADCSTART);

	/* wait for the ADC to become ready */
	timeout = 0;
	while (!(RD(ADC_ACTL3) & ACTL_ADCRDY)) {
		if (++timeout > 2000000) {
			return -1;
		}
	}

	/* sequencer 0: software trigger, 3 samples of channel 63 */
	WR(ADC_EMUX, 0);                                /* EM0 = software */
	WR(ADC_SEQ0_SCCTL, 0);                          /* ICNT=0, RCNT=0 */
	WR(ADC_SEQ0_SRTMR, 0);
	WR(ADC_SEQ0_SRQCTL, 2);                         /* RQMAX=2 (3 samples) */
	WR(ADC_SEQ0_SRQSEL0,
	    TSENSOR_CH | (TSENSOR_CH << 8) | (TSENSOR_CH << 16));
	WR(ADC_CHDELAY63, HSECLK_HZ / 10000);           /* extra sample delay */

	WR(ADC_SEQSYNC, ADC_SEQSYNC_SYNC0);
	WR(ADC_SEQEN, ADC_SEQEN_SEQEN0);

	return 0;
}

static void tsensor_init(void) {
	WR(SIU_TSENSCTRL, TSENSCTRL_SCALE);             /* enable, scaled */
}

static uint16_t adc_read_tsensor(void) {
	uint32_t s[3];
	int i;

	WR(ADC_IC, ADC_RIS_SEQRIS0);                    /* clear pending */
	/* keep SYNC0 (software sync enable) set, pulse GSYNC to trigger */
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
		fprintf(stderr, "tsensor: ADC did not become ready\n");
		return -1;
	}
	tsensor_init();

	printf("tsensor: ADC ch%d (internal temp sensor), %ld sample%s\n",
	    TSENSOR_CH, n, n == 1 ? "" : "s");

	for (i = 0; i < n; i++) {
		uint16_t raw = adc_read_tsensor();
		uint32_t mv = (uint32_t)raw * AVCC_MV / 4096;
		int32_t t = (int32_t)((int64_t)574713 * (793 - (int32_t)mv) / 100000);

		printf("[%ld] raw=%u  V=%lu mV  T=%ld.%ld C\n",
		    i + 1, (unsigned)raw, (unsigned long)mv,
		    (long)(t / 10), (long)(t % 10));

		if (i + 1 < n) {
			usleep(500 * 1000);
		}
	}

	return 0;
}
