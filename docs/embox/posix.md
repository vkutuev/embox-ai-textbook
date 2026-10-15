# POSIX Layer

Embox provides a POSIX-compatible layer, so ordinary Linux C programs often
compile and run with little or no change. The board examples are plain POSIX
programs using `printf`, `open`/`read`/`write`, `sleep`, `usleep`, `gmtime_r`,
and `mktime`.

## Practical notes

- **Calendar time.** `mktime`/`gmtime_r`/`strptime` live in
  `embox.compat.posix.time.time`; pull it in when a command or driver needs
  calendar time (the niiet RTC driver depends on it).
- **Floating-point `printf`.** `printf` float support is a build option
  (`support_floating`); the ADC examples avoid `%f` by computing in fixed
  point.
- **Device files.** Drivers expose devices under `/dev` (e.g. the serial
  driver provides `/dev/ttyS<N>`, flash `/dev/stm32_flash`) and are accessed
  with ordinary POSIX calls.
- **Surface.** Embox ships a broad libc and ports of common software (Qt,
  Dropbear SSH, PJSIP); see [Overview](overview.md).

## Sources

- Embox wiki: [Home](https://github.com/embox/embox/wiki)
- Embox source: `src/compat/posix/`
