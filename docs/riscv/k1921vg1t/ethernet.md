# Ethernet

A 10/100/1000 Mbit/s Ethernet MAC at `5000_B000h` with an RGMII interface and
MDIO management. It uses a descriptor-ring model: separate transmit and
receive descriptor tables referenced by base-address registers. On the dev
board it is paired with an external PHY (the `EXTPHY` option selects the PHY
layer).

## Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CTRL | control and capabilities |
| +04h | STATUS | status |
| +08h | MACMSB | MAC address high bytes |
| +0Ch | MACLSB | MAC address low bytes |
| +10h | MDIOCSR | MDIO control and status (PHY management) |
| +14h | TXBASE | transmit descriptor-table base address |
| +18h | RXBASE | receive descriptor-table base address |
| +40h | RGMII_STAT | RGMII status |
| +44h | RGMII_IF | RGMII interrupt flags |
| +48h | RGMII_IM | RGMII interrupt mask |
| +4Ch | WRAP_100M | RGMII 25 MHz clock wrap |
| +50h | EDGE1_100M | RGMII 25 MHz first edge |
| +54h | EDGE2_100M | RGMII 25 MHz second edge |
| +58h | WRAP_10M | RGMII 2.5 MHz clock wrap |
| +5Ch | EDGE1_10M | RGMII 2.5 MHz first edge |
| +60h | EDGE2_10M | RGMII 2.5 MHz second edge |
| +64h | BIT_PATTERN | RGMII bit pattern |
| +68h | NEG_DELAY | RGMII negative-clock data delay |
| +6Ch | POS_DELAY | RGMII positive-clock data delay |
| +70h | BIT_SWAP | RGMII sample-order swap |
| +74h | DATA_SWAP | RGMII input bus swap |
| +78h | CTRL_SWAP | RGMII control-signal swap |
| +7Ch | PHYCFG | PHY control (EXTPHY) |

## Operation

The MAC is programmed like a typical descriptor-based Ethernet controller:
software fills transmit/receive descriptor tables in RAM and points `TXBASE`/
`RXBASE` at them; the hardware DMAs packets to and from the buffers. The MAC
address is set via `MACMSB`/`MACLSB`, and the external PHY is configured over
MDIO through `MDIOCSR`. The RGMII register group adjusts clock/data timing
and pin mapping for the PHY wiring.

## Embox driver

No Embox driver yet. Ethernet is a larger bring-up (PHY init + descriptor
DMA) and is not yet covered in the examples.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (Ethernet chapter).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`ETH_TypeDef`) in
  <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
