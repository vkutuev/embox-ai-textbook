# RTC example for NIIET-DEV-K1921VG1T

Bring up the VG1T real-time clock (backed by the 32.768 kHz crystal and
the CR1220 battery) and use it through Embox's RTC subsystem.

## Hardware

- Insert a **CR1220** cell into the board's battery holder (or supply
  `V_BAT` on XP3 pin 37). This keeps the RTC running when the board's
  main power is off. No other wiring — the crystal is on the board.

## Install into Embox

1. Copy `rtc/` into `<embox>/src/drivers/rtc/niiet/` (the driver).
2. Add the three `include` lines from `mods.conf.snippet` to `mods.conf`
   (the RTC framework, the niiet driver, and the stock `rtc` command).
3. Build, flash, reset.

## Run and verify

```
root@embox:/#rtc                          # show current time
root@embox:/#rtc -s 2026:10:14:12:30:00   # set the time
root@embox:/#rtc                          # confirm it advances
```

**Battery persistence**: set the time, then remove the board's 12 V power
for a few seconds and re-apply it. Run `rtc` again — the time kept
running on the battery.

## How it works

The driver implements Embox `rtc_ops` (`get_time`/`set_time`, converting
`struct tm` ↔ `RTC.TIME` Unix seconds) and registers with `RTC_DEVICE_DEF`.
Init enables the RTC clock (`RCU.CGCFGAPB0`/`RSTDISAPB0.RTCEN`), sets the
`RTCCFG` divider, resets it, and switches to the crystal (`CONTROL.DRC`)
once `STATUS.OSCOK` reports the oscillator is running.
