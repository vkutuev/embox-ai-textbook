# Errata

Version от 27.02.2026. Items relevant to system software development:

## 1. I2S may capture the right channel of the first frame

Probabilistic. Workaround: send control words in the first frames to
synchronize receiver and transmitter.

## 2. DMA error-logger registers not reset on AXI error

Always. The `CH[i].ERR_RD_ADDR` / `ERR_WR_ADDR` logger registers are not
cleared on an AXI slave/decode error. Workaround: clear them by writing
`0x1E` to `CH[i].INT_CLEAR`, or leave them — the next error overwrites the
value.

## 3. DMA error-address capture on first/last AXI transaction

Always. On an AXI slave/decode error in the first or last transaction, the
wrong transaction address may be captured. Fix expected in a future MCU
revision.

## 4. DMA watchdog keeps counting on AXI error

Always. With `CH[i].ERR_STOP` set, the watchdog still counts after an AXI
error. Workaround: in the interrupt handler stop the channel
(`CH[i].CH_ACTIVE = 0`) or reset it (`CH_ENABLE &= ~(1 << i)`).

## 5. DMA hang on 4 KiB address boundary crossing

Always. A DMA transfer hangs when it crosses a 4 KiB boundary with an
unaligned AXI burst. Workarounds:

1. Align source/destination buffers or force 32-bit bursts:
   `CH[i].STATIC0_bit.RD_BURST_MAX = 4;`
   `CH[i].STATIC1_bit.WR_BURST_MAX = 4;`
2. Enable the timeout interrupt (`CH[i].INT_ENABLE_bit.WDT_TIMEOUT = 1`)
   and stop/reset the stuck channel in the handler.

## 6. DMA hang with mixed shared/normal channel modes

Always. If some channels work in shared mode while others work in normal
mode, DMA may hang. Workarounds: as in item 5, or in the timeout handler
run a high-priority dummy transfer on a free channel.

## 7. Spurious MSC upstream_time_out interrupt

Probabilistic. Occurs right after writing `U_CTRL.TOV`/`U_CTRL.TOP` when
`RBR`/`TOP` previously held non-zero values. Workaround: use an 8-bit write
to clear `U_CTRL` before setting the new value (disables upstream first).

## 8. Increased VBAT consumption

In samples of 2026 before correction, VBAT/XI_RTC/XO_RTC pins are powered
from AVDD33, drawing ~9 mA on VBAT when the main supply is off. Workaround:
use high-capacity batteries (CR2477, CR2450, or 2x AA/AAA in series).

## 9. ADC zero offset

Sometimes. ADC channels show a zero offset of up to 100-150 mV. Workaround:
manual calibration to -64 after ADC initialization, plus set
`CH_DELAY[] = Fadc / 10000` for each measured channel:

```c
while (!(ADC->ACTL[i].ACTL_bit.ADCRDY)) {}
ADC->ACTL[i].ACTL_bit.CALEN  = 1;
ADC->ACTL[i].ACTL_bit.CALIN  = 0x40;   /* -64 */
ADC->ACTL[i].ACTL_bit.CALLOAD = 1;
```

## 10. Weak Ethernet PHY link pulses

Always. NLP amplitude is insufficient; the remote receiver may not detect
link pulses. Investigation ongoing; workaround: use an external Ethernet
PHY connected via GMII.

## Sources

- `doc/K1921VG1T/Errata_K1921VG1T.pdf` in
  <https://gitflic.ru/project/niiet/niiet>
