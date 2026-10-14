# RTC

The VG1T has a real-time clock (base `5000_5000h`) that keeps a 32-bit
**seconds counter** (Unix time). On the dev board it runs from a
32.768 kHz crystal and is backed by the VBAT domain, so with a CR1220
cell in the battery holder it keeps time while the main power is off.

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | TIME | seconds counter (read/write) |
| +04h | ALARM1 | alarm 1 compare (seconds) |
| +08h | ALARM2 | alarm 2 compare |
| +0Ch | CONTROL | `A1IE`/`A2IE` (alarm IRQ), `DRC` (bit 8, disable RC → use crystal), `FCM` (bit 9, 1 Hz vs 32 kHz tick) |
| +10h | STATUS | `OSCOK` (bit 8, oscillator running), `BU` (bit 31, battery in use), alarm flags |

## Clock and reset

- Enable the peripheral clock with `RCU.CGCFGAPB0.RTCEN` and release
  reset with `RCU.RSTDISAPB0.RTCEN` (bit 1).
- `RCU.RTCCFG` (`C4h`): `DIVN` (divider, ~system clock / 40 MHz),
  `DIVEN`, `RSTDIS`.
- The `RSTDIS` pulse resets the RTC (and **clears `TIME`**), so software
  must save `TIME`, reset, and restore it to survive a reset.
- Once `STATUS.OSCOK` reports the crystal is running, set `CONTROL.DRC`
  to switch from the RC clock to the crystal.

## Embox driver

Embox has an RTC framework (`src/drivers/rtc/`: `rtc_ops`,
`rtc_get_device`, `rtc_get_time`/`rtc_set_time`, `RTC_DEVICE_DEF`) and a
stock `rtc` command (`rtc`, `rtc -s Y:M:D:H:M:S`, `rtc -a <sec>`). The
[niiet RTC driver](КФДЛ.441461.050/example-rtc.md) implements
`get_time`/`set_time` (`struct tm` ↔ `TIME` via `gmtime_r`/`mktime`) and
registers with `RTC_DEVICE_DEF` — the stock command then works unchanged.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (RTC chapter).
- SDK `projects/NIIET-DEV-K1921VG1T/rtc` in
  <https://gitflic.ru/project/niiet/niiet_riscv_sdk>.
