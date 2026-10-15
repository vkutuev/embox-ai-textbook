# CAN

The microcontroller provides one CAN 2.0B controller with four nodes and four
independent CAN FD controllers (CANFD0-CANFD3).

## CAN 2.0B

One controller (`4101_0000h`) containing four CAN 2.0B nodes (11/29-bit
identifiers, up to 1 Mbit/s). It uses a message-object model: each node owns
message-list, pending, and identifier banks, plus per-node registers.

### Register map

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CLC | frequency control |
| +08h | ID | identity (module ID) |
| +0Ch | FDR | clock divider |
| +10h | LIST[8] | message-list registers |
| +34h | MSPND[8] | message-pending registers |
| +58h | MSID[8] | message-identifier registers |
| +7Ch | MSIMASK | message-index mask |
| +80h | PANCTR | command panel control |
| +84h | MCR | control |
| +88h | MITR | interrupt |
| +0C0h | Node[4] | per-node register set |

Each node exposes its own register block for message objects, acceptance
filtering, error counters, and bit timing.

## CAN FD

Four identical CAN FD controllers based on the Bosch M_CAN message-object
model (MOb), with bit-rate switching and a shared timestamp.

Base addresses:

```
CANFD0  4100_0000h   CANFD1  4100_1000h   CANFD2  4100_2000h   CANFD3  4100_3000h
```

### Register map (per controller)

| Offset | Register | Description |
| ------ | -------- | ----------- |
| +00h | CONTROL | general control |
| +04h | STATUS | general status |
| +08h | IF | interrupt flags |
| +0Ch | IE | interrupt enables |
| +10h | MOBEN | enable message objects |
| +14h | MOBIE | enable MOb interrupts |
| +18h | MOBIF | MOb interrupt flags |
| +1Ch | BITTIMING | nominal bit timing |
| +20h | TIMERCTRL | timestamp control |
| +24h | TIMERVAL | timestamp value |
| +28h | TTCVAL | trigger-timestamp capture |
| +2Ch | TREC | transmit error counter |
| +30h | HPRIMOB | highest-priority MOb |
| +38h | FD_CTRL | FD mode control |
| +3Ch | MOBFDEN | MOb FD-mode enable |
| +40h | FDBITTIMING | FD (data-phase) bit timing |
| +6Ch | MOBPAGE | MOb page select |
| +70h | TIMESTAMP | MOb timestamp |
| +74h | MOBSTATUS | MOb status |
| +78h | MOBCTRL | MOb control + DLC |
| +7Ch | IDTAG | MOb identifier |
| +80h | IDMASK | MOb identifier mask |
| +84h | MESSAGE[16] | MOb data (16 × 32-bit words) |

## Embox driver

Embox has a niiet CAN driver (see the board
[development notes](КФДЛ.441461.050/development-notes.md)). Bringing up CAN
on the board requires a transceiver; check loopback mode first.

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet> (CAN and CAN FD chapters).
- `platform/Device/K1921VG1T/Include/K1921VG1T.h` (`CAN_TypeDef`,
  `CANFD_TypeDef`) in <https://gitflic.ru/project/niiet/k1921vg1t_sdk>.
