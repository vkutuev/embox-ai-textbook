# Example: LED/button counter

Press the USER button — a counter increments and the four LEDs show
its value in binary.

> **Status: verified on hardware (2026-10-10)** — the polling variant
> works. The IRQ and input-subsystem variants hit a GPIO interrupt
> quirk on this board (interrupt storm on press / at boot) and are
> documented in the
> [experiment journal](experiment-led-button-counter.md).

## Hardware

| Signal | Pin | Notes |
| ------ | --- | ----- |
| USER button | GPIOB[3] | **verified**: press = low, idle high |
| LED0..LED3 | GPIOC[12..15] | **verified**: **active-high** (pin high = LED on) |

The mapping comes from the SDK BSP for this board
(`hardware/bsp/NIIET-DEV-K1921VG1T/bsp.h`, `bsp.c`). Note the BSP's
`BSP_LED_On()` clears `DATAOUT` bits, which implies active-low — but
the real board's LEDs are active-high (verified with the `pin`
command). The button init (`BSP_Btn_Init`) is a TODO in the BSP;
polarity was verified by reading `pin GPIOB 3 get` while pressing.

## Embox support

The `k1921vg1t_dev` template already enables the GPIO driver:

```
include embox.driver.gpio.niiet_gpio(altfunc_field_width=4)
```

Ports A–G (16-bit each) are described in
`platform/niiet/board_config/niiet_k1921vg1t_dev.conf.h`, and the
standard API (`src/drivers/gpio/gpio.h`) is available:

- `gpio_setup_mode(port, pins, mode)` — input/output/pull/IRQ/altfunc
- `gpio_set(port, pins, level)` / `gpio_toggle(port, pins)` /
  `gpio_get(port, pins)`
- `gpio_irq_attach(port, pins, handler, data)`

The `pin` shell command (already built into the template) is handy for
bring-up: `pin GPIOC 12 reset` lights LED0, `pin GPIOB 3 get` reads the
button.

Note that `gpio_set()` applies a **single level to a whole pin mask** —
there is no per-bit value write, so displaying a pattern needs two
calls (all off, then "on" bits low).

## Variants

The example exists in three forms, all in
[`examples/niet-dev-k1921vg1t/led_button_counter/`](../../../../examples/niet-dev-k1921vg1t/led_button_counter/):

| Variant | Command | How the button is read | Status |
| ------- | ------- | ---------------------- | ------ |
| Polling | `led_counter_poll` | `gpio_get()` in a 10 ms loop with software debounce | **verified** |
| Interrupt | `led_counter_irq` | GPIO interrupt on the falling edge (press); main loop applies events | root cause found, needs driver patches — see below |
| Input subsystem | `led_counter_input` | reads `struct input_event` from `/dev/input/user-bt` produced by the `gpio_button` driver | breaks boot, see below |

Comparison:

- **Polling** is the simplest to read and debug and cannot lose the
  CPU to a noisy pin; it wastes a little CPU time. Verified working.
- **Interrupt** shows the RTOS/IRQ path and does not poll. On this
  board it is currently unusable; the root cause is identified and
  two Embox patches are provided in
  [`examples/.../led_button_counter/patches/`](../../../../examples/niet-dev-k1921vg1t/led_button_counter/patches/):
  (1) `niiet_gpio` never records the attached IRQ handler, so repeated
  `gpio_setup_mode()` calls fail with `-EBUSY` and silently disable
  the interrupt; (2) the RISC-V trap handler nests interrupts during
  dispatch, so a permanently-asserted GPIO line smashes the stack.
  Even with both patches the `INTSTATUS` latch on this silicon keeps
  the line asserted, so the CPU spins while pressed. See the
  [experiment journal](experiment-led-button-counter.md).
- **Input subsystem** is the most idiomatic Embox approach, but
  `gpio_button` configures a both-edges interrupt on the idle-high
  button pin, so `INTSTATUS` latches at enable and the PLIC storms
  during module init — the boot hangs at runlevel 0. The template
  keeps this module commented out for a reason; enabling it needs a
  driver fix, not a config change.

## Build and run

1. Copy the example directory into an Embox checkout:

   ```console
   $ cp -r examples/niet-dev-k1921vg1t/led_button_counter/ <embox>/src/cmds/example/
   ```

2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, inside the
   `conf` block, add the include lines for **one** variant (see
   `mods.conf.snippet` in the example directory). The input variant
   also needs:

   ```
   include embox.driver.input.button.gpio_button(pin_port=1, pin_num=3)
   ```

3. Build, flash, and reset as in the
   [board guide](README.md) (`make confload-platform/niiet/k1921vg1t_dev`,
   `make -j16`, then flash `build/base/bin/embox`).

4. Run the command from the shell:

   ```
   root@embox:/#led_counter_poll
   led_counter_poll: press USER to count (Ctrl+C to stop)
   counter = 1
   counter = 2
   ...
   ```

Expected behavior: LED0..LED3 display the counter in binary (LED0 =
bit 0), the counter wraps from 15 to 0, and every change is printed to
the console.

## Verification checklist (run on hardware, 2026-10-10)

1. **LEDs**: `pin GPIOC 12 reset` → LED0 off; `pin GPIOC 12 set` →
   LED0 on. Walking-1 over pins 13–15 confirmed the LED0..LED3 order.
   Result: **active-high**, pins 12–15. ✔
2. **Button**: `pin GPIOB 3 get` while pressing → low; released →
   high. Result: **press = low**. ✔
3. Floating pins light their LEDs (inputs float high) — the example
   drives all four LED pins explicitly, which is required.
4. `led_counter_poll`: verified — one increment per press, wrap at 15.
5. `led_counter_irq` / `led_counter_input`: **failing** on this board
   (interrupt storm) — see the
   [experiment journal](experiment-led-button-counter.md).

## Sources

- NIIET SDK BSP for the board:
  `hardware/bsp/NIIET-DEV-K1921VG1T/{bsp.h,bsp.c}`
  (<https://gitflic.ru/project/niiet/k1921vg1t_sdk.git>)
- Embox GPIO API: `src/drivers/gpio/gpio.h`
  (<https://github.com/embox/embox>)
- Embox GPIO driver for NIET MCUs:
  `src/drivers/gpio/niiet/niiet_gpio/`
- Embox button input driver:
  `src/drivers/input/button/gpio_button/`
