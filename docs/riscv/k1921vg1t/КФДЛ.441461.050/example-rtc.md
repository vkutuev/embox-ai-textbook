# Example: RTC (real-time clock)

Bring up the VG1T RTC and use it through Embox's RTC subsystem and the
stock `rtc` command. The CR1220 battery keeps the time across power-off.

> **Status: verified on hardware (2026-10-14)** — `rtc -s` sets the time,
> `rtc` reads it back, it persists across a reset, and it keeps time on
> the battery. See the [experiment journal](experiment-rtc.md).

## Hardware

Insert a **CR1220** cell into the board's battery holder (X6) **and
install a jumper on the "BAT ON" connector (XP6)**. Without the jumper,
the MCU's `VBAT` pin routes to `V_BAT` on XP3 pin 37 (external battery
input), so the onboard cell does nothing.

## Build and run

1. Copy `examples/niet-dev-k1921vg1t/rtc/` into `<embox>/src/drivers/rtc/niiet/`.
2. Add the four `include` lines from `mods.conf.snippet` to `mods.conf`
   (RTC framework, niiet driver, stock `rtc` command, and the
   `embox.compat.posix.time.time` module for `mktime`/`gmtime_r`).
3. Build, flash, reset.
4. On the console:

   ```
   root@embox:/#rtc                          # show current time
   Current RTC time: 1970- 1-01 00:00:09
   root@embox:/#rtc -s 2026:10:14:12:30:00   # set the time
   root@embox:/#rtc                          # confirm it advances
   Current RTC time: 2026-10-14 12:30:01
   ```

## Verification checklist (on hardware)

1. `rtc -s` sets the time and `rtc` reads it back. ✔ (2026-10-14)
2. The time persists across a reset. ✔
3. (Battery) Remove the 12 V power for a few seconds, re-apply, and
   confirm `rtc` still shows the running time. ✔

## How it works

The driver implements Embox `rtc_ops` (`get_time`/`set_time`, converting
`struct tm` ↔ `RTC.TIME` seconds) and registers with `RTC_DEVICE_DEF`.
Init enables the RTC clock, sets the `RTCCFG` divider, saves `TIME`,
resets the RTC, switches to the crystal (`CONTROL.DRC`) once `OSCOK`
reports it, then restores `TIME`.

## Sources

- RTC chapter: [rtc.md](../rtc.md).
- Embox RTC framework: `src/drivers/rtc/`.
