# Interrupts

Embox routes external interrupts through a generic API. A driver or command
attaches a handler to an IRQ line; the handler runs in interrupt context and
reports whether it handled the interrupt.

## API

Header: `kernel/irq.h`

```c
typedef enum {
	IRQ_NONE    = 0,   /* not handled */
	IRQ_HANDLED = 1,   /* handled */
} irq_return_t;

typedef irq_return_t (*irq_handler_t)(unsigned int irq_nr, void *data);

int irq_attach(unsigned int irq_nr, irq_handler_t handler,
		unsigned int flags, void *data, const char *dev_name);
int irq_detach(unsigned int irq_nr, void *data);
```

- `irq_attach()` registers `handler` for IRQ `irq_nr`. `data` is passed back to
  the handler, `dev_name` is a human-readable owner name, and `flags` accepts
  `IF_SHARESUP` for shared lines.
- The handler must return `IRQ_HANDLED` (1) if it processed the interrupt, or
  `IRQ_NONE` (0) otherwise.
- `irq_detach()` removes the handler (matched by `irq_nr` + `data`).

## Gotchas

- **Re-attach leak.** `irq_attach()` does not replace an existing handler: a
  second attach to the same IRQ fails ("cannot attach"). Call
  `irq_detach(irq_nr, dev_id)` before re-attaching — the GPIO, timer, and DMA
  examples all hit this on command re-runs.
- **Return value.** Handlers must return `IRQ_HANDLED`, not 0, or the kernel
  treats the interrupt as spurious.

At the application level, device interrupts are usually set up through the
subsystem API (e.g. `gpio_irq_attach()` — see [Drivers](drivers.md)) rather
than `irq_attach()` directly.

## Sources

- Embox source: `src/include/kernel/irq.h`
- Embox wiki: [GPIO](https://github.com/embox/embox/wiki/GPIO)
