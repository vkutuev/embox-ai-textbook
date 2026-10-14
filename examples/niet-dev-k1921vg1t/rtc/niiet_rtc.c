/**
 * @file
 * @brief NIIET K1921VG1T RTC driver (implements Embox rtc_ops).
 *
 * The VG1T RTC (0x50005000) keeps a 32-bit seconds counter (Unix time),
 * backed by a 32.768 kHz crystal on the board and the VBAT (CR1220)
 * battery when the main power is off.
 *
 * Registers: TIME (+0), ALARM1 (+4), ALARM2 (+8), CONTROL (+0x0C:
 * DRC bit 8 = use crystal, FCM bit 9 = 1 Hz tick), STATUS (+0x10:
 * OSCOK bit 8 = crystal oscillator OK).
 */

#include <stdint.h>
#include <time.h>

#include <drivers/rtc.h>
#include <embox/unit.h>

#define RTC_TIME      0x50005000UL
#define RTC_CONTROL   0x5000500CUL
#define RTC_STATUS    0x50005010UL

#define RCU_CGCFGAPB0   0x50004004UL
#define RCU_RSTDISAPB0  0x50004014UL
#define RCU_RTCCFG      0x500040C4UL

#define RCU_RTCEN        (1u << 1)
#define RTCCFG_RSTDIS    (1u << 4)
#define RTCCFG_DIVEN     (1u << 12)
#define RTCCFG_DIVN(v)   ((v) << 16)

#define RTC_CONTROL_DRC    (1u << 8)
#define RTC_STATUS_OSCOK   (1u << 8)

#define RD(reg)           (*(volatile uint32_t *)(reg))
#define WR(reg, val)      (*(volatile uint32_t *)(reg) = (val))

static int niiet_rtc_get_time(struct rtc_device *rtc, struct tm *tm) {
	time_t t = (time_t)RD(RTC_TIME);

	gmtime_r(&t, tm);

	return 0;
}

static int niiet_rtc_set_time(struct rtc_device *rtc, struct tm *tm) {
	time_t t = mktime(tm);

	WR(RTC_TIME, (uint32_t)t);

	return 0;
}

static struct rtc_ops niiet_rtc_ops = {
	.get_time = niiet_rtc_get_time,
	.set_time = niiet_rtc_set_time,
};

static struct rtc_device niiet_rtc_dev = {
	.name = "niiet",
	.rtc_ops = &niiet_rtc_ops,
};

RTC_DEVICE_DEF(&niiet_rtc_dev);

static int niiet_rtc_init(void) {
	uint32_t saved;
	volatile uint32_t i;

	/* enable the RTC peripheral clock and release reset */
	WR(RCU_CGCFGAPB0, RD(RCU_CGCFGAPB0) | RCU_RTCEN);
	WR(RCU_RSTDISAPB0, RD(RCU_RSTDISAPB0) | RCU_RTCEN);

	/* RTC clock divider (system clock -> ~40 MHz); 1 Hz tick */
	WR(RCU_RTCCFG, RTCCFG_DIVN(4) | RTCCFG_DIVEN);

	saved = RD(RTC_TIME);   /* save the counter before reset */

	/* reset the RTC to re-sync the divider (this clears the counter) */
	WR(RCU_RTCCFG, RD(RCU_RTCCFG) | RTCCFG_RSTDIS);
	for (i = 0; i < 10000; i++) {
		__asm__ __volatile__("nop");
	}
	WR(RCU_RTCCFG, RD(RCU_RTCCFG) & ~RTCCFG_RSTDIS);

	/* switch to the 32.768 kHz crystal once it is running */
	if (RD(RTC_STATUS) & RTC_STATUS_OSCOK) {
		WR(RTC_CONTROL, RD(RTC_CONTROL) | RTC_CONTROL_DRC);
	}

	/* restore the counter so a reset / power-cycle keeps the time */
	WR(RTC_TIME, saved);

	return 0;
}

EMBOX_UNIT_INIT(niiet_rtc_init);
