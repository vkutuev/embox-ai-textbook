# HASH

A hash processor at `4106_0000h` that computes SHA-1, SHA-224, SHA-256, and
MD5 digests in hardware. A related crypto block (`4107_0000h`, `CRYPTO`)
provides AES-128/256 and the Russian Kuznechik/Magma ciphers.

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CR | control: algorithm selection, mode, data-order control |
| +04h | DATAIN | data input word |
| +08h | STR | start: NBLW (valid bits in the last word), DCAL (start digest calc) |
| +0Ch | IMR | interrupt mask (DINIE, DCIE) |
| +10h | SR | status: DINIS, DCIS, DMAS, BUSY |
| +40h | HR[8] | digest registers (8 × 32-bit = up to 256 bits) |

## Operation

1. Configure the algorithm in `CR` (and clear it between calculations).
2. Write message words to `DATAIN` (the block accepts one 32-bit word per
   write, with DMA support).
3. Write the number of valid bits in the last word to `STR.NBLW` and set
   `STR.DCAL` to start the digest calculation.
4. Poll `SR.DCIS` (or use the interrupt/DMA), then read the digest from the
   `HR[0..7]` registers — SHA-256 uses all 8, SHA-1/MD5 use the first 4-5.

`SR.BUSY` indicates a calculation in progress; `SR.DINIS`/`DCIS` are the
input-ready and calculation-complete interrupts.

## Embox driver

No Embox driver yet; programmed directly. See the board
[development notes](КФДЛ.441461.050/development-notes.md) for the
zero-hardware CRC/HASH example.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (HASH and CRYPTO chapters).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`HASH_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
