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

## Not yet verified

- **Battery persistence**: removing the 12 V power (so the RTC runs from
  the CR1220) was not tested here — that is the user's power-cycle step.
  The reset persistence (3.3 V rail) is confirmed.

## Environment notes

- The CH340 console corrupts long output; the `rtc` date/time output is
  partially garbled but the minute:second is readable.
