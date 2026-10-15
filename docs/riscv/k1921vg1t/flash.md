# Flash Memory

The microcontroller has three non-volatile areas: main flash (FlashM),
data flash (FlashD), and OTP, all with Hamming ECC.

## FlashM (4 MiB)

- Address range `0000_0000h - 003F_FFFFh`; 4 banks x 1 MiB, 2048 pages of
  2 KiB.
- Read through the AXI bus; a write to the address range returns a slave
  error.
- Program/erase via controller registers (base `5000_2000h`): `DATAn`
  (n=0-3), `ADDR`, `CMD` (key `0xc0de`), `STAT`. Write only to erased
  cells; erase per bank or per page. Full erase = four bank-erase commands.
- Erase time up to 20 ms per page; programming 16 bytes takes ~20 µs.
- Read latency: set additional wait states via the `LAT` field of `CTRL`
  depending on SYSCLK:

| SYSCLK (max, MHz) | Extra wait states |
| ----------------- | ----------------- |
| 204 | 5 |
| 175 | 4 |
| 140 | 3 |
| 105 | 2 |
| 70 | 1 |
| 35 | 0 |

(Value after reset is 1.)

## FlashD (512 KiB)

- Address range `0040_0000h - 0047_FFFFh`; 512 pages of 1 KiB.
- Program via `DATAn` (n=0-1), `ADDR`, `CMD`, `STAT` (base `5000_C000h`).
- Programming 8 bytes takes ~20 µs; erase up to 20 ms.

## OTP (16 KiB)

- 16 pages of 1 KiB inside the FlashD controller; read/write-once through
  the FlashD register interface with the `NVRON` bit of `CMD` set. Write
  only to cells that still read `0xFFFFFFFF`.

## NVR area and configuration word (CFGWORD)

A 128 KiB NVR region of FlashM is accessible only through the FLASHM
register interface. The configuration word is read after every power-on
reset from the last NVR cell at address `003F_FFF0h`:

| Field | Bits | Description |
| ----- | ---- | ----------- |
| SWMODEEN | 4 | software mode change (LOCKSTEP/DUALCORE) allowed (default 1) |
| LOCKSTEPEN | 3 | start in LOCKSTEP mode after POR (default 1) |
| JTAGEN | 2 | JTAG debug enabled (default 1) |
| FLASHWE | 1-0 | flash write protection: 01 = write/erase forbidden, 11 = unprotected (default) |

## Service (full) erase

- Hold `SERVEN` high (pull to 3.3 V) during reset: all flash reads return
  zero and the JTAGEN bit is ignored.
- Over JTAG, write the `SERVCMD` bit of the SIU `SERVCTL` register four
  times (once per FlashM bank, changing the bank address each time). All
  main and boot areas are erased (except OTP).
- If a full erase is not needed, `SERVEN` must be low during reset.

## Flashing without a debug adapter (UART bootloader)

The very first flash needs JTAG/OpenOCD, but after a one-time install of
the NIIET UART bootloader ("Flasher") the chip can be programmed over
UART with no debug adapter at all.

- Bootloader: <https://gitflic.ru/project/niiet/k1921vkx_flasher> —
  Python tool `app/k1921vx_flasher.py` (plus `.exe` releases), prebuilt
  `.bin` images in `bin/`, `.elf` in `firmware/k1921vgxt/releases`. The
  K1921VG1T is covered by `README_VGxT.md` (shared with VG3T/VG5T/VG7T);
  the wire protocol is in `NOTES.md`.
- It is **not** a factory ROM bootloader: it must be flashed once via
  OpenOCD/JTAG (`make load`). Once installed, the MCU passes through it
  on every power-on.
- Interface: UART0 (**TX = B1, RX = B0**, 8N1, auto-baud). Boot-selection
  pin **BOOTEN = A8**.
- Entry: pull BOOTEN low (e.g. USB-UART **RTS**), reset the MCU, connect
  with the tool, then release BOOTEN. On the dev board the CH340B
  provides RTS (BOOTEN) and DTR (reset) for exactly this flow.
- The application is linked at `0x0000_2000` (the bootloader occupies
  `0x0000_0000 - 0x0000_1FFF` and protects its own pages); use the
  `k1921vg1t_flash_bl.ld` linker script.
- Supported commands: page read/write/erase (main and NVR areas), full
  erase, read/modify CFGWORD. Pages occupied by the bootloader are
  protected from read/erase/write.

## ECC

Flash ECC corrects one error and detects two (64 data bits protected by 8
ECC bits). Enable with the `ECCEN` bit of `FECCCR`; single/double error
counters `FECCSEC`/`FECCDEC`, addresses `FECCSEA`/`FECCDEA`, interrupts via
`ISEUEN`/`IDEUEN`.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>, chapter 9
- UART bootloader ("Flasher"): <https://gitflic.ru/project/niiet/k1921vkx_flasher>
  (`README_VGxT.md`, `NOTES.md`)
