# Drivers

This page summarizes the user and driver APIs for the most common peripheral
drivers: GPIO, I2C, flash, and framebuffer.

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

## Sources

- [embox-docs: GPIO](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_gpio_en.md)
- [embox-docs: I2C](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_i2c_en.md)
- [embox-docs: Flash devices](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_flash_en.md)
- [embox-docs: Frame buffer](https://github.com/embox/embox-docs/blob/master/en/drivers/embox_drivers_framebuffer_en.md)
