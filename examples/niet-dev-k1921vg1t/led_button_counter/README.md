# LED/button counter example for NIIET-DEV-K1921VG1T

An Embox example for the КФДЛ.441461.050 (NIIET-DEV-K1921VG1T) board:
press the USER button and the counter increments; the four LEDs show
the counter value in binary (0..15, wraps around).

> **Status: not yet verified on hardware.** The board was disconnected
> when this example was written. Pin/level facts below come from the
> NIIET SDK BSP and must be confirmed on the real board.

## Board resources

| Signal | Pin | Notes |
| ------ | --- | ----- |
| USER button | GPIOB[3] | polarity unverified; examples assume press = low |
| LED0..LED3 | GPIOC[12..15] | **active-low**: output bit cleared = LED on |

The mapping is from the SDK BSP
(`hardware/bsp/NIIET-DEV-K1921VG1T/bsp.h`, `bsp.c`):
`BSP_LED_On` clears `DATAOUT` bits, so the LEDs are active-low. The
button init in the BSP is a TODO, which is why its polarity is listed
as unverified.

## Files

| File | Purpose |
| ---- | ------- |
| `board.h` | shared port/pin defines and `leds_write()` helper |
| `led_counter_poll.c` | polling + software debounce variant |
| `led_counter_irq.c` | GPIO interrupt variant |
| `led_counter_input.c` | input subsystem (`/dev/input/user-bt`) variant |
| `Mybuild` | Embox module declarations for all three commands |
| `mods.conf.snippet` | `include` lines for each variant |

## Install into Embox

1. Copy this directory into an Embox checkout:

   ```console
   $ cp -r led_button_counter/ <embox>/src/cmds/example/
   ```

2. In `platform/niiet/templates/k1921vg1t_dev/mods.conf`, inside the
   `conf` block, add the include line for the variant you want (see
   `mods.conf.snippet`). The input variant needs the `gpio_button`
   driver as well.

3. Build Embox with the `k1921vg1t_dev` template (see the board guide
   for the toolchain setup), flash the image, and run the command
   (`led_counter_poll`, `led_counter_irq`, or `led_counter_input`)
   from the shell.

## Expected behavior

- On start: all LEDs off, `counter = 0`.
- Each press: counter increments by 1, the value is shown in binary
  on the LEDs (LED0 = bit 0) and printed to the console.
- After 15 the counter wraps to 0.
- Exit with Ctrl+C if the shell supports it; otherwise reboot.

## Polarity notes

- **LEDs**: active-low is handled once, in `leds_write()`.
- **Button**: the examples configure an internal pull-up and treat
  low as pressed. If the board wires the button as active-high, either
  invert the press test, or (IRQ variant) change
  `GPIO_MODE_INT_RISING_FALLING` handling in the handler.
- `gpio_set()` applies a *single level to a whole pin mask* — this is
  why `leds_write()` does two calls (all off, then "on" bits low).
