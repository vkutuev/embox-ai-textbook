# Experiment: RTC (2026-10-14)

Hardware verification of the [RTC example](example-rtc.md) on the
КФДЛ.441461.050 board (J-Link V9 + CH340 console).

## Result

**Verified.** The niiet RTC driver works with the stock `rtc` command:
`rtc -s` sets the time, `rtc` reads it back, and the time persists
across a reset.

## Findings

- This is a proper Embox **driver** (implements `rtc_ops` + registers
  with `RTC_DEVICE_DEF`), unlike the other examples which program the
  registers from a command. The stock `rtc` command works unchanged.
- `mktime`/`gmtime_r` are in `embox.compat.posix.time.time`, which had to
  be added (the RTC framework's own timekeeping also needs them).
- **Reset clears `TIME`**: the SDK's init saves the counter, pulses
  `RCU.RTCCFG.RSTDIS`, and restores it. The first attempt omitted the
  save/restore, so every boot reset the time to 1970 — fixed.
- The driver switches to the 32.768 kHz crystal (`CONTROL.DRC`) once
  `STATUS.OSCOK` reports it.

## Battery persistence

- **Verified**: with the CR1220 in the holder and a jumper on **XP6
  ("BAT ON")**, removing the 12 V power for ~10 s and re-applying kept
  the time running.
- **XP6 jumper**: without it, the MCU's `VBAT` pin is routed to `V_BAT`
  on XP3 pin 37 (external battery input), so the onboard cell does not
  back up the RTC. XP6 also lets you measure `VBAT` current (remove the
  jumper, insert an ammeter).

## Environment notes

- The CH340 console corrupts long output; the `rtc` date/time output is
  partially garbled but the minute:second is readable.
