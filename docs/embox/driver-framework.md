# Driver Framework

This page describes how Embox drivers are structured at the framework level:
the device classes, their registration macros, and the interfaces a new
driver implements. It complements [Commands and drivers](commands-drivers.md)
(generic module pattern) and [Drivers](drivers.md) (user-facing APIs).

## Driver classes

Embox groups drivers by device class:

| Class | Purpose |
| --- | --- |
| `diag` | early-boot console output (before interrupts) |
| interrupt | interrupt controllers |
| clock | timers that generate system ticks |
| serial | UART devices |
| network | Ethernet devices |
| gpio / spi / i2c | GPIO, SPI, I2C buses |
| video | framebuffer |
| audio / usb / pci | audio, USB, PCI |

## Diagnostic interface (`diag`)

The diagnostic interface provides the earliest possible console output via
`printk()`/`kprintf()` and the logging functions (`log_error`, `log_info`,
`log_debug`). It works without interrupts; if no full tty is configured, it
degrades to a polled basic tty. There is at most one diag device.

A driver implements `struct diag_ops`:

```c
struct diag_ops {
    int  (*init)(const struct diag *diag);
    char (*getc)(const struct diag *diag);
    void (*putc)(const struct diag *diag, char ch);
    int  (*kbhit)(const struct diag *diag);
};

DIAG_OPS_DEF(
    .putc = diag_mem_putc,
);
```

It is selected in configuration:

```
include embox.driver.diag(impl="embox__driver__serial__i8250")
```

## Interrupt controller

A controller driver extends `embox.driver.interrupt.irqctrl_api` and
registers itself with `IRQCTRL_DEF(name, init)`:

```c
module cortexm_nvic extends embox.driver.interrupt.irqctrl_api { ... }

static int nvic_init(void) { ... }

IRQCTRL_DEF(cortexm_nvic, nvic_init);
```

The initializer runs before the module initializers (`kernel_init()`). The
core requirement is enabling/disabling an interrupt by its system number:

```c
extern void irqctrl_enable(unsigned int nr);
extern void irqctrl_disable(unsigned int nr);
```

Application code attaches handlers with `irq_attach()` (see
[Interrupts](interrupts.md)).

## Clock source

Timers are exposed through `CLOCK_SOURCE_DEF()`, which registers a
`time_counter_device` (a free-running counter) and/or a
`time_event_device` (an interrupt-generating timer):

```c
struct time_counter_device {
    uint32_t cycle_hz;
    uint64_t mask;                          /* max counter value */
    cycle_t (*read)(struct clock_source *cs);
};

struct time_event_device {
    int (*set_oneshot)(struct clock_source *cs);
    int (*set_periodic)(struct clock_source *cs);
    int (*set_next_event)(struct clock_source *cs, uint32_t next_event);
    uint32_t flags;
    volatile clock_t jiffies;
    uint32_t event_hz;
};

CLOCK_SOURCE_DEF(integratorcp, integratorcp_cs_init, NULL,
    &integratorcp_event_device, &integratorcp_counter_device);
```

The event handler must call `clock_tick_handler()` with the `clock_source`
that raised the interrupt.

## Input devices

Input devices (`drivers/input/input_dev.h`) deliver `struct input_event`
events. Consumers read them either through the file interface
(`read()` on `/dev/<name>`) or the `input_dev` API:

```c
struct input_dev *indev = input_dev_lookup("somedev");
input_dev_open(indev, indev_cb);   /* events arrive in the callback */
input_dev_close(indev);
```

A driver registers a device with `input_dev_register()` and reports events
from its interrupt handler with `input_dev_report_event(dev, &ev)`. Driver
options are read with `OPTION_GET` (`<framework/mod/options.h>`):

```c
#include <framework/mod/options.h>
#define SOMEDEV_PARAM OPTION_GET(NUMBER, param1)
```

## Ethernet

An Ethernet driver registers a `struct net_device` with
`inetdev_register_dev()`. The device is allocated with `etherdev_alloc()`
and its operations are provided as a `struct net_driver`:

```c
struct net_driver {
    int (*start)(struct net_device *dev);
    int (*stop)(struct net_device *dev);
    int (*xmit)(struct net_device *dev, struct sk_buff *skb);
    int (*set_macaddr)(struct net_device *dev, const void *addr);
    int (*mdio_read)(struct net_device *dev, uint8_t reg);
    int (*mdio_write)(struct net_device *dev, uint8_t reg, uint16_t data);
    void (*set_phyid)(struct net_device *dev, uint8_t phyid);
    int (*set_speed)(struct net_device *dev, int speed);
};
```

Received packets are wrapped in `struct sk_buff` and handed to the stack
with `netif_rx()`; transmitted packets are released with `skb_free()`.

## Sources

- Embox user manual (Russian), generated from
  <https://github.com/embox/embox-docs/tree/master/ru> (the "Драйвера"
  chapter).
- Embox source: `src/drivers/`, `src/include/drivers/`
