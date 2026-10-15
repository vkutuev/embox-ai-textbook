# Canonical References

Curated list of upstream sources for the K1921VG1T / КФДЛ.441461.050 / Embox
material. Cite these (never local scratch files) in documentation and in agent
instructions.

## Embox

| Resource | URL |
| --- | --- |
| Main repository | <https://github.com/embox/embox> |
| Documentation (English) | <https://github.com/embox/embox-docs> |
| Wiki | <https://github.com/embox/embox/wiki> |
| Releases | <https://github.com/embox/embox/releases> |
| Prebuilt toolchains (crosstool) | <https://github.com/embox/crosstool> |
| Prebuilt toolchain archives | <https://github.com/embox/crosstool/releases> |

## Cross-compilation toolchains

The K1921VG1T core is RV32IMAFDCP; Embox builds it with a RISC-V
`riscv64-unknown-elf-*` toolchain. Any of the following works:

| Option | Source |
| --- | --- |
| Syntacore Development Toolkit (SC-DT) | <https://syntacore.com/tools/development-tools> |
| Embox prebuilt RISC-V toolchain (2.44-14.3.0-15.2) | <https://github.com/embox/crosstool/releases/tag/2.44-14.3.0-15.2> |
| OS package manager (e.g. `gcc-riscv64-unknown-elf`) | distribution-specific |
| Build from source (CrosstoolNG / Buildroot) | <https://github.com/riscv-collab/riscv-gnu-toolchain> |

## NIIET (НИИЭТ) documentation

| Resource | URL |
| --- | --- |
| Documentation repository (user manual, errata, quick-start PDFs) | <https://gitflic.ru/project/niiet/niiet> |
| User manual `doc/K1921VG1T/UM_K1921VG1T_10YA.pdf` | in the repository above |
| Errata `doc/K1921VG1T/Errata_K1921VG1T.pdf` | in the repository above |
| Quick-start `doc/Start_NIIET_RISCV.pdf` | in the repository above |

## NIIET OpenOCD

| Resource | URL |
| --- | --- |
| OpenOCD fork with the K1921VG1T flash driver | <https://gitflic.ru/project/niiet/openocd> |
| Release with patches | <https://gitflic.ru/project/niiet/openocd/release/4a4e7431-0990-455c-9a8b-487bc98f72f8> |

## NIIET SDKs

| Resource | URL |
| --- | --- |
| Per-MCU SDK (device headers, linker scripts, OpenOCD configs) | <https://gitflic.ru/project/niiet/k1921vg1t_sdk> |
| NIIET RISC-V SDK (startup, SVD files, examples) | <https://gitflic.ru/project/niiet/niiet_riscv_sdk.git> |

## Board hardware

| Resource | URL |
| --- | --- |
| КФДЛ.441461.050 board documentation (rev 1.1) | <https://gitflic.ru/project/niiet_hardware/441461_050> |

## RISC-V

| Resource | URL |
| --- | --- |
| RISC-V GNU toolchain | <https://github.com/riscv-collab/riscv-gnu-toolchain> |
| Homebrew RISC-V (macOS) | <https://github.com/riscv-software-src/homebrew-riscv> |
