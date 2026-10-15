# Commands and Drivers

How to add a **command** (a program the shell runs) or a **driver** (a device
the kernel manages) to Embox. Both are ordinary Mybuild modules; they differ
only in their annotations and how they are registered.

## Commands

A command is a module whose `main()` is invoked from the shell, registered
with `@AutoCmd` and described by `@Cmd`. Example:

```
package embox.cmd.example

@AutoCmd
@Cmd(name = "hello_world", help = "First Embox application",
     man = "'''Prints Hello, world!'''")
module hello_world {
	source "hello_world.c"
	depends embox.compat.libc.stdio
}
```

The `@Cmd` annotation declares how the command appears in `help`:

- `name` — the command name (required).
- `help` — a one-line description (default `"TODO no help available"`).
- `man` — a longer manual page (default `"TODO no man page available"`).

`@AutoCmd` makes the command appear in the shell automatically. The `depends`
clause names the module whose exported header the source includes; observed
mappings for the K1921VG1T examples: GPIO → `embox.driver.gpio.core`, I2C →
`embox.driver.i2c_core`, PWM → `embox.driver.pwm.pwm_dev`, DMA →
`embox.driver.dma.dma_new`, RTC → `embox.driver.rtc.rtc`.

## Drivers

A driver implements a subsystem interface and publishes a device. It has no
`main()`; instead it exports headers via `@IncludeExport` and runs an
initializer via `EMBOX_UNIT_INIT`.

```c
#include <drivers/rtc.h>
#include <embox/unit.h>

static int my_get_time(struct rtc_device *rtc, struct tm *tm) { ... }
static int my_set_time(struct rtc_device *rtc, struct tm *tm) { ... }

static struct rtc_ops my_ops = {
	.get_time = my_get_time,
	.set_time = my_set_time,
};

static struct rtc_device my_dev = {
	.name = "my",
	.rtc_ops = &my_ops,
};

RTC_DEVICE_DEF(&my_dev);

static int my_init(void) {
	/* enable the clock, release reset, configure registers */
	return 0;
}

EMBOX_UNIT_INIT(my_init);
```

- `EMBOX_UNIT_INIT(fn)` registers `fn` to run once at boot (`EMBOX_UNIT` and
  `EMBOX_UNIT_FINI` add a teardown callback).
- The subsystem-specific `*_DEVICE_DEF` macro (e.g. `RTC_DEVICE_DEF`) publishes
  the device to the subsystem's device registry.
- `@IncludeExport(path = "...", target_name = "...")` controls how a module's
  header is exported to dependents (e.g. the DMA header is consumed as
  `<drivers/dma.h>`).

The driver's `Mybuild` lives in its package (e.g. `package embox.driver.rtc.niiet`)
and declares `depends` on the interface module it implements.

## Building with a new module

1. Place the module sources in the source tree.
2. Add `include <package>.<module>` to `conf/mods.conf`.
3. Rebuild with `make`.

See [Build system](build-system.md) for the configuration flow and
[Quick start](quick-start.md) for the first Hello World.

## Sources

- Embox wiki: [Mybuild](https://github.com/embox/embox/wiki/Mybuild),
  [Mybuild Annotations](https://github.com/embox/embox/wiki/Mybuild-Annotations)
- Embox source: `mk/mybuild/lang/Cmd.my`, `src/include/embox/unit.h`
