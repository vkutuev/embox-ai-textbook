# Watchdog (WDT)

A single watchdog timer based on the ARM PrimeCell SP805: a 32-bit down
counter that interrupts and then resets the system if software stops
reloading it. Base address `5000_6000h`.

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | LOAD | reload value written to the counter on timeout/reload |
| +04h | VALUE | current counter value (read) |
| +08h | CTRL | control: INTEN (enable interrupt on timeout), RESEN (enable reset output) |
| +0Ch | INTCLR | write 1 to clear the watchdog interrupt |
| +10h | RIS | raw interrupt status |
| +14h | MIS | masked interrupt status |
| +C00h | LOCK | write `0x1ACCE551` to unlock; any other value locks register writes |
| +C00h | LOCKSTAT | read back: REGWRDIS set when writes are locked |

## Operation

Write the reload value to `LOAD`, enable `CTRL.RESEN` (and `CTRL.INTEN` for a
pre-reset interrupt), and the counter starts counting down from `LOAD`. Every
write to `LOAD` restarts the count. If the counter reaches zero, the watchdog
signals an interrupt (`INTEN`) and, after a second timeout, asserts the reset
output (`RESEN`).

To prevent the reset, the application must reload `LOAD` periodically within
the timeout. The timeout is `LOAD / fWDTCLK` (the watchdog clock comes from
the RCU; reset the block via the RCU before use).

## Locking

The register block is protected by a software lock. Write `0x1ACCE551` to
`LOCK` to allow writes to the other registers; write any other value to lock
them again. `LOCKSTAT.REGWRDIS` reads `1` while locked.

## Embox driver

No Embox driver yet; the watchdog is programmed directly. A watchdog-reset
demo is listed as a candidate example (zero external hardware) in the board
[development notes](КФДЛ.441461.050/development-notes.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (WDT chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`WDT_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
