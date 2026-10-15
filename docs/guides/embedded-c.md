# Embedded C

Programming a microcontroller means reading and writing **peripheral
registers**, which are special memory locations mapped to hardware. This page
introduces the C techniques behind that: memory-mapped I/O, `volatile`, bit
operations, and register maps.

## Memory-mapped I/O

A peripheral register is just an address. To the CPU, writing `0x1` to address
`0x52010000` means "drive this UART line high" rather than "store a byte in
RAM". The hardware decodes the address and acts on it. This is
**memory-mapped I/O (MMIO)**: the same load/store instructions that access
RAM also control peripherals.

## `volatile`

A compiler normally assumes memory only changes when the program writes it,
so it may reorder, cache, or even remove "redundant" accesses. Peripheral
registers change *by themselves* (a status bit set by hardware), so the
compiler must not optimize away those accesses. Mark such pointers/objects
`volatile`:

```c
volatile uint32_t *reg = (volatile uint32_t *) 0x52010000;
*reg = 0x1;   /* always emitted, never cached or reordered */
```

Rule of thumb: **any access to a peripheral register goes through a
`volatile` pointer.**

## Bit operations

Registers pack many independent fields into one word, so you manipulate
individual bits:

```c
/* Set (write 1 to) a bit */
reg |=  (1u << bit);

/* Clear (write 0 to) a bit */
reg &= ~(1u << bit);

/* Toggle a bit */
reg ^=  (1u << bit);

/* Test a bit (non-zero if set) */
if (reg & (1u << bit)) { ... }

/* Replace a multi-bit field */
reg = (reg & ~(MASK << shift)) | (value << shift);
```

## Read-modify-write discipline

A register usually holds several fields. Changing one field must preserve the
others, so you **read** the current value, **modify** the bits you care about,
and **write** the whole word back:

```c
reg = (reg & ~FIELD_MASK) | (new_value << FIELD_SHIFT);
```

Writing a freshly computed value (e.g. `reg = (1u << ENABLE)`) instead of a
read-modify-write **clears every other field** — a classic embedded bug. The
K1921VG1T notes record a real case: the ADC `SEQSYNC` trigger must be written
as `SYNC0 | GSYNC`, because writing only `GSYNC` clears `SYNC0` and silently
disables the trigger.

## Register maps

Rather than sprinkling magic addresses through the code, define a register map
— either a struct per peripheral or named macros/constants:

```c
typedef struct {
    volatile uint32_t DATA;
    volatile uint32_t STATUS;
    volatile uint32_t CTRL;
} uart_regs_t;

#define UART0 ((uart_regs_t *) 0x52010000)

UART0->CTRL |= (1u << 2);   /* enable TX */
```

The SDK device headers (from the NIIET SDK) already provide such definitions
for the K1921VG1T peripherals; the examples in this repository use the Embox
driver APIs instead, where possible.

## `static` globals and re-runs

On a bare-metal command that runs repeatedly, a `static` global keeps its
value across runs. Reset per-run state in `main()` rather than relying on
static initialization. See the board's
[development notes](../riscv/k1921vg1t/КФДЛ.441461.050/development-notes.md).

## Sources

- `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` in
  <https://gitflic.ru/project/niiet/niiet>
- [RISC-V unprivileged specification](https://riscv.org/technical/specifications/)
