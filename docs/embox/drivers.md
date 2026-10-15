# Drivers

This page summarizes the user and driver APIs for the most common peripheral
drivers: GPIO, I2C, flash, framebuffer, PWM, DMA, RTC, and serial.

## GPIO

Header: `drivers/gpio/gpio.h`

### User API

```c
int gpio_setup_mode(unsigned short port, gpio_mask_t pins, int mode);
void gpio_set(unsigned short port, gpio_mask_t pins, char level);
void gpio_toggle(unsigned short port, gpio_mask_t pins);
gpio_mask_t gpio_get(unsigned short port, gpio_mask_t pins);
int gpio_irq_attach(unsigned short port, gpio_mask_t pins,
    irq_handler_t pin_handler, void *data);
```

- `port` — combination of GPIO port and GPIO controller number; with a single
  controller, just `GPIO_PORT_B` etc.
- `pins` — bitmask of pins.
- `mode` — `GPIO_MODE_OUTPUT`, `GPIO_MODE_INPUT`,
  `GPIO_MODE_INT_MODE_RISING`, etc.
- `gpio_setup_mode()` returns 0 on success or a negative value if the port is
  not available.
- `gpio_irq_attach()` registers an interrupt handler; the pins must first be
  configured for interrupts with `gpio_setup_mode()`.

Example — toggle an LED with a button on STM32F4-Discovery:

```c
#include <unistd.h>
#include <kernel/irq.h>
#include <drivers/gpio/gpio.h>

#define LED4_PIN        (1 << 12)
#define USER_BUTTON_PIN (1 << 0)

irq_return_t user_button_hnd(unsigned int irq_nr, void *data) {
    gpio_toggle(GPIO_PORT_D, LED4_PIN);
    return IRQ_HANDLED;
}

int main(int argc, char *argv[]) {
    gpio_setup_mode(GPIO_PORT_D, LED4_PIN, GPIO_MODE_OUTPUT);
    gpio_setup_mode(GPIO_PORT_A, USER_BUTTON_PIN, GPIO_MODE_INT_MODE_RISING);

    if (0 > gpio_irq_attach(GPIO_PORT_A, USER_BUTTON_PIN, user_button_hnd, NULL)) {
        return -1;
    }

    sleep(30);

    return 0;
}
```

### Driver API

Header: `drivers/gpio/gpio_driver.h`

Each GPIO driver defines a `struct gpio_chip` with function pointers
`setup_mode`, `set`, `get`, and the `nports` field (number of GPIO ports of
the controller), then registers it:

```c
int gpio_register_chip(struct gpio_chip *gpio_chip, unsigned char chip_id);
```

Drivers notify the global GPIO subsystem about interrupts with:

```c
void gpio_handle_irq(struct gpio_chip *gpio_chip, unsigned int irq_nr,
    unsigned char port, gpio_mask_t changed_pins);
```

Supported controllers: STM32, i.MX6, QEMU ARM OMAP3 Overo, Bifferboard.

## I2C

### User API

```c
ssize_t i2c_bus_read(int id, uint16_t addr, uint8_t *ch, size_t sz);
ssize_t i2c_bus_write(int id, uint16_t addr, const uint8_t *ch, size_t sz);
```

- `id` — bus number
- `addr` — chip address on the bus
- Returns the number of bytes transferred or a negative error code.

Calling `i2c_bus_write()` with `sz == 0` probes whether a device is present,
so it can be used to scan available devices.

### Driver API

To add I2C support for a new platform, define a `struct i2c_adapter` and pass
it to `i2c_bus_register()`. The operations are described in
`struct i2c_algorithm` via the `i2c_master_xfer` routine. The direction of an
operation is encoded in the `flags` field of `struct i2c_msg`: if
`I2C_M_RD` is set, it is a read; otherwise a write.

Supported adapters: STM32, i.MX6.

## Flash devices

Header: `drivers/flash/flash.h`

### User API

```c
struct flash_dev *flash_by_id(int idx);

int flash_read(struct flash_dev *flashdev, unsigned long offset,
    void *buf, size_t len);
int flash_write(struct flash_dev *flashdev, unsigned long offset,
    const void *buf, size_t len);
int flash_erase(struct flash_dev *flashdev, uint32_t block);
int flash_copy(struct flash_dev *flashdev, uint32_t to,
    uint32_t from, size_t len);
```

Flash can also be accessed via devfs using regular POSIX calls; block erasure
is done with `ioctl`:

```c
int fd = open("/dev/stm32_flash", O_RDWR);

flash_getconfig_erase_t arg = {
    .offset = 0xbeef0000,
    .len    = 0x1000
};

ioctl(fd, GET_CONFIG_FLASH_ERASE, &arg);
```

### Driver API

To implement another flash driver:

1. Implement the operations of `struct flash_dev_drv` (`flash_init`,
   `flash_query`, `flash_erase_block`, `flash_program`, `flash_read`,
   `flash_copy`).
2. Create the device with `flash_create()`.

An example driver implementation is
`src/drivers/flash/emulator/emulator.c`. Supported controllers:
STM32F3Discovery, STM32F4Discovery, and a software emulator.

## Frame buffer

Header: `drivers/video/fb.h`

### User API

```c
struct fb_info *fb = fb_lookup(0);   /* id 0 for a single display */
```

Screen resolution and other information come from `fb->var`
(`struct fb_var_screeninfo`: `xres`, `yres`, `bits_per_pixel`, `vmode`,
`fmt`). Frame data is mapped at `fb->screen_base`:

```c
/* Write a white pixel at (i, j), assuming a 32-bit RGB format */
((uint32_t *) fb->screen_base)[j * fb->var.xres + i] = 0xffffffff;
```

Supported pixel formats: RGB888, BGR888, RGBA8888, BGRA8888, RGB565, BGR565.
Helper functions: `pix_fmt_has_alpha()`, `pix_fmt_bpp()`,
`pix_fmt_chan_bits()`, `pix_fmt_chan_get_val()`, `pix_fmt_chan_set_val()`,
`pix_fmt_convert()`.

### Driver API

A video driver for a new platform implements `fb_set_var` and `fb_get_var` in
a `struct fb_ops` and creates the framebuffer with:

```c
struct fb_info *new_fb = fb_create(&ops, screen_base, screen_size /* bytes */);
```

Supported controllers: STM32F7Discovery, i.MX6 LVDS, EFM32 LCD display,
PL110 for IntegratorCP, BOCHS, Cirrus.

## PWM

Header: `drivers/pwm.h`

### User API

```c
struct pwm_device *pwm_dev_by_id(int id);
int pwm_set_frequency(struct pwm_device *pwm, int freq_hz);
int pwm_set_duty(struct pwm_device *pwm, int channel, int duty_ns);
int pwm_enable(struct pwm_device *pwm, unsigned long channels);
```

- `pwm_dev_by_id()` looks up a PWM controller by id; it returns `NULL` if
  the controller is not enabled in the configuration.
- `pwm_set_frequency()` sets the period (derived from the controller clock).
- `pwm_set_duty()` sets the high time of a channel **in nanoseconds** (not a
  percentage; values below 2 ns are rejected).
- `pwm_enable()` takes a **bitmask** of channels (`1U << chan`).

Board usage:
[PWM example](../riscv/k1921vg1t/КФДЛ.441461.050/example-pwm.md).

## DMA

Header: `drivers/dma.h` (the "new" DMA API; exported as `<drivers/dma.h>`,
not `<drivers/dma/dma_new.h>`).

### User API

```c
struct dma_dev *dma_dev_by_id(int id);
int dma_init(struct dma_dev *dev);
int dma_config(struct dma_dev *dev, int chan, struct dma_config *conf);
int dma_transfer(struct dma_dev *dev, int chan, struct dma_req *req);
int dma_activate(struct dma_dev *dev, int chan);
int dma_get_type(struct dma_dev *dev, int type);
```

A transfer is described by `struct dma_req`: source/destination address
(`dr_src`/`dr_dest`), transfer width (`dr_src_width`/`dr_dest_width`),
increment flags (`dr_src_inc`/`dr_dest_inc`), requestor type
(`dr_src_type`/`dr_dest_type`), size (`dr_size`), and an optional completion
callback (`dr_callback`, signature `int cb(struct dma_req *, void *, int)`).

`dma_config()` attaches the channel interrupt (which leaks across command
re-runs — detach it first), `dma_transfer()` programs the transfer, and
`dma_activate()` starts it. Board usage:
[DMA copy example](../riscv/k1921vg1t/КФДЛ.441461.050/example-dma.md).

## RTC

Header: `drivers/rtc.h`

### Driver API

An RTC driver implements `struct rtc_ops` (`get_time`, `set_time`, each
taking a `struct tm *`) and registers a `struct rtc_device` with
`RTC_DEVICE_DEF(&dev)`; the initializer runs via `EMBOX_UNIT_INIT`. Calendar
conversion helpers (`gmtime_r`, `mktime`, `strptime`) live in
`embox.compat.posix.time.time`.

Board usage:
[RTC example](../riscv/k1921vg1t/КФДЛ.441461.050/example-rtc.md) — the niiet
driver wraps the 32-bit seconds counter at `0x50005000`.

## Serial (UART)

The `pl011` driver exposes UARTs as POSIX character devices (`/dev/ttyS<N>`).
Applications use ordinary `open`/`read`/`write`; enabling
`embox.driver.serial.pl011_ttyS<N>` registers the device and configures the
pins and clock.

```c
int fd = open("/dev/ttyS1", O_RDWR);
read(fd, buf, sizeof(buf));
write(fd, buf, n);
```

Board usage:
[UART1 echo example](../riscv/k1921vg1t/КФДЛ.441461.050/example-uart1-echo.md).

The SPI `pl022` driver (`embox.driver.spi.pl022_spi`) follows the same
per-device pattern; a loopback example is planned.

## Sources

- [embox-docs: GPIO](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_gpio_en.md)
- [embox-docs: I2C](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_i2c_en.md)
- [embox-docs: Flash devices](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_flash_en.md)
- [embox-docs: Frame buffer](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_framebuffer_en.md)
