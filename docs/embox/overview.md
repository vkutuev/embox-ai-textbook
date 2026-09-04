# Embox Overview

Embox is a real-time operating system for embedded systems.

## Key features

### Cross-platform

All architecture-dependent code is organized as separate modules, which makes
porting to a new platform easy. Embox supports the following architectures:
ARM, x86, RISC-V, SPARC, Microblaze, MIPS, PPC, and E2k (Elbrus).

### Multitasking

Embox supports different priority levels, preemptive and cooperative
multitasking, priority inheritance, and different synchronization primitives.

### POSIX compatibility

Embox provides a POSIX-compatible layer, which allows to use a lot of existing
Linux software, for example Qt Embedded, the Dropbear SSH server, and PJSIP.

### Modularity and configurability

Embox is designed as a modular and configurable project. During the
configuration stage one chooses which modules will be included in the final
image, so it has low resource requirements. The declarative Mybuild language
describes both single modules and the whole target system.

### Security

Statically building the image ensures that functionality that was not included
in the target image cannot be executed.

### Standard toolset

The development process is common for all platforms, including
resource-restricted ones such as MCUs, because of the use of a standard
toolset (make, gcc, cross-compilers, GDB).

## Use cases

- **Embedded systems** — the main scope of the project.
- **IoT** — Embox has a complete TCP/IP stack and a rich set of user software
  with low resource requirements.
- **Robotics** — combines rich functionality and real-time tasks in one
  system. POSIX applications can control servos, motors, and sensors while
  communicating over the network. Embox has been ported to Lego Mindstorms
  NXT.
- **Hardware design** — as a highly configurable system, Embox runs in very
  constrained environments such as hardware simulators, so it can be used at
  early stages of hardware development to estimate required resources.
- **Certification** — the build system produces a source distribution with
  only the required modules, reducing the amount of code to certify; it can
  also provide a full file list and a module graph.
- **Education** — used for teaching embedded systems, robotics, OS
  architecture, and telecommunication systems.

## Sources

- [embox-docs: Quick Overview](https://github.com/embox/embox-docs/blob/master/en/embox_quick_overview_en.md)
- [Embox wiki: Home](https://github.com/embox/embox/wiki/Home)
- [Archived wiki: QuickOverview (Google Code)](https://web.archive.org/web/20160105033137/https://code.google.com/p/embox/wiki/QuickOverview?wl=en)
