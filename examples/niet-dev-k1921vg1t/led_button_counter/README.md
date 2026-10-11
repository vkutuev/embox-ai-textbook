# LED/button counter example for NIIET-DEV-K1921VG1T

An Embox example for the КФДЛ.441461.050 (NIIET-DEV-K1921VG1T) board:
press the USER button and the counter increments; the four LEDs show
the counter value in binary (0..15, wraps around).

> **Status (2026-10-12): verified on hardware.** LEDs LED1–LED4 on
> GPIOC12–15 (active-high), USER button on GPIOB3 (press = low). Both
> the **polling** and **IRQ** variants work (five presses → `counter =
> 1..5`). The input variant still hits a GPIO interrupt quirk at boot.

## Board resources

| Signal | Pin | Notes |
| ------ | --- | ----- |
| USER button | GPIOB[3] | verified: high when released, low while pressed |
| LED1..LED4 | GPIOC[12..15] | **verified: active-high** (pin high = LED on) |

The pin mapping is from the SDK BSP
(`hardware/bsp/NIIET-DEV-K1921VG1T/bsp.h`, `bsp.c`). Note the BSP's
`BSP_LED_On()` clears `DATAOUT` bits, which implies active-low — but
on the real board the LEDs are active-high (verified with the `pin`
command). The board silkscreen labels the LEDs LED1–LED4 (GPIOC12 =
LED1, … GPIOC15 = LED4). The button init in the BSP is a TODO;
polarity was verified by reading `pin GPIOB 3 get` while pressing.

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
  on the LEDs (LED1 = bit 0) and printed to the console.
- After 15 the counter wraps to 0.
- Exit with Ctrl+C if the shell supports it; otherwise reboot.

## Polarity notes

- **LEDs**: active-high, handled once in `leds_write()`. The SDK BSP
  code implies the opposite (`BSP_LED_On()` clears the output bit);
  trust the empirical result.
- **Button**: press = low, confirmed on hardware. The examples
  configure an internal pull-up and treat low as pressed.
- `gpio_set()` applies a *single level to a whole pin mask* — this is
  why `leds_write()` does two calls (all off, then "on" bits high).

## Known issues on this board

- `led_counter_irq` works, but **do not re-run `gpio_setup_mode()`**
  on the button port after configuration: each call re-writes the
  edge-configuration registers and latches a spurious `INTSTATUS` on
  this silicon, storming the PLIC. The example masks/clears/re-arms
  via the `INTENCLR`/`INTSTATUS`/`INTENSET` registers directly (see
  [`patches/`](patches/)).
- `led_counter_input` requires the `gpio_button` driver, whose
  both-edges IRQ configuration latches `INTSTATUS` at boot and hangs
  the system at module init.
- `niiet_gpio` has an IRQ-attach bookkeeping bug (never records the
  attached handler) — see `patches/0001`.
- Full GDB evidence: the board guide's experiment journal.
