# Embox AI Textbook

Textbook for microcontroller programming with the
[Embox](https://github.com/embox/embox) operating system. Covers Embox itself
and running it on specific microcontrollers and development boards.

## Documentation Layout

| Path                                  | Purpose                              |
| ------------------------------------- | ------------------------------------ |
| `docs/<arch>/<microcontroller>/<board>` | Board-specific documentation, e.g. `docs/riscv/k1921vg1t/КФДЛ.441461.050` |
| `docs/<arch>/<microcontroller>/`      | Microcontroller-level documentation (core, memory map, peripherals) |
| `docs/embox/`                         | Architecture-agnostic Embox documentation |
| `docs/guides/`                        | Architecture-agnostic learning material for newcomers (Git, cross-compilation, embedded C, JTAG/OpenOCD/GDB, RISC-V) |
| `docs/references.md`                  | Canonical list of upstream sources |
| `workdir/`                            | User workspace (gitignored): clone Embox/OpenOCD/SDKs here |

## Agent Instructions

Repository-wide rules for AI agents live in [AGENTS.md](AGENTS.md):

## License

[Creative Commons Attribution 4.0 International](LICENSE)
