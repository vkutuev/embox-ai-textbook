# Embox

Architecture-agnostic documentation for the
[Embox](https://github.com/embox/embox) operating system, distilled from the
official Embox documentation and wiki.

## Contents

- [Overview](overview.md) — what Embox is, its key features and use cases
- [Quick start](quick-start.md) — environment setup, building and running on
  QEMU, first application, debugging
- [Build system](build-system.md) — Mybuild language, modules, configurations,
  make targets
- [Drivers](drivers.md) — GPIO, I2C, flash, framebuffer, PWM, DMA, RTC,
  and serial APIs
- [Commands and drivers](commands-drivers.md) — writing commands (`@Cmd`)
  and drivers (`*_DEVICE_DEF`, `EMBOX_UNIT_INIT`)
- [Driver framework](driver-framework.md) — device classes, `diag`,
  interrupt controller, clock source, input, Ethernet
- [Interrupts](interrupts.md) — `irq_attach`/`irq_detach` API and gotchas
- [POSIX layer](posix.md) — libc surface, time, device files
- [Platforms](platforms.md) — supported CPU architectures and boards
- [Porting](porting.md) — porting Embox to a new CPU architecture

## Conventions

Documents derived from upstream sources carry a **Sources** footer linking to
the original pages in
[embox/embox-docs](https://github.com/embox/embox-docs) (English version) and
the [Embox wiki](https://github.com/embox/embox/wiki). Never cite local
scratch files as sources: always link the canonical upstream page.
