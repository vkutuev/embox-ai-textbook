# Example: LED/button counter

Press the USER button — a counter increments and the four LEDs show
its value in binary.

> **Status: not yet verified on hardware.** Written while the board was
> disconnected; pin and level facts below come from the NIIET SDK BSP
> and must be confirmed on the real board (see the verification
> checklist at the end).

## Hardware

| Signal | Pin | Notes |
| ------ | --- | ----- |
| USER button | GPIOB[3] | polarity unverified; the examples assume press = low |
| LED0..LED3 | GPIOC[12..15] | **active-low**: output bit cleared = LED on |

The mapping comes from the SDK BSP for this board
(`hardware/bsp/NIIET-DEV-K1921VG1T/bsp.h`, `bsp.c`): `BSP_LED_On()`
clears `DATAOUT` bits, so the LEDs are active-low; the button init
(`BSP_Btn_Init`) is a TODO in the BSP, which is why the button polarity
is listed as unverified.

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

| Variant | Command | How the button is read |
| ------- | ------- | ---------------------- |
| Polling | `led_counter_poll` | `gpio_get()` in a 10 ms loop with software debounce |
| Interrupt | `led_counter_irq` | GPIO interrupt on both edges; only the edge into the pressed state is counted |
| Input subsystem | `led_counter_input` | reads `struct input_event` from `/dev/input/user-bt` produced by the `gpio_button` driver |

Comparison:

- **Polling** is the simplest to read and debug and cannot lose the
  CPU to a noisy pin; it wastes a little CPU time.
- **Interrupt** shows the RTOS/IRQ path and does not poll; it needs
  correct edge configuration and care about switch bounce.
- **Input subsystem** is the most idiomatic Embox approach: a
  reusable button *driver* (`gpio_button`, already scaffolded but
  commented out in the template with `pin_port=1, pin_num=3`) reports
  events to any consumer. Note that `gpio_button` reports
  `USER_BUTTON_PRESSED` when the pin reads *high*; with an active-low
  button the "pressed" event fires on release — the counter still
  increments once per press cycle.

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

## Verification checklist (to run on hardware)

1. **LEDs**: `pin GPIOC 12 reset` → LED0 lights; `pin GPIOC 12 set` →
   LED0 off. Repeat for pins 13–15. Confirms port, pins, and active
   level.
2. **Button**: `pin GPIOB 3 get` while pressing and releasing the
   USER button. Confirms polarity (low = pressed vs. high = pressed)
   and whether an internal pull-up is enough or an external resistor
   is present.
3. If polarity differs from the assumption, fix `board.h`/the press
   test and note the finding.
4. Run each variant; confirm one increment per press (no bounce
   artifacts) and the wrap at 15.
5. Record results in the experiment journal, then update this guide
   (status → verified, polarity confirmed).

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
